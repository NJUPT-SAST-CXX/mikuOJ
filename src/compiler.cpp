#include "cppjudge/compiler.h"

#include "cppjudge/language.h"
#include "cppjudge/log.h"

#include <sys/stat.h>

#include <fstream>
#include <sstream>

namespace cppjudge {

namespace {

// 读取编译器 stdout/stderr，用于 CE 详情或诊断。
std::string read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// 把原始提交复制到本次判题 work_dir。
//
// 复制动作由 Compiler 负责，而不是由 Sandbox Core 负责。Sandbox Core 只接收
// 已经准备好的 work_dir，并把它作为可写目录挂载到沙箱内的 /box。
bool copy_file(const std::string& from, const std::string& to, std::string& err) {
    std::ifstream src(from, std::ios::binary);
    if (!src.is_open()) { err = "cannot open source file: " + from; return false; }
    std::ofstream dst(to, std::ios::binary);
    if (!dst.is_open()) { err = "cannot write to work dir: " + to; return false; }
    dst << src.rdbuf();
    return dst.good();
}

// 编译产物检查。编译器 exit_code 为 0 但产物不存在时，仍按 CE 处理，并给出
// sandbox verdict / exit_code / signal 作为诊断线索。
bool file_exists(const std::string& path) {
    struct stat st{};
    return stat(path.c_str(), &st) == 0;
}

} // namespace

CompileResult Compiler::compile(SandboxBackend& backend,
                                const std::string& source_file,
                                Language lang,
                                const std::string& work_dir) {
    CompileResult r;

    // Language Manager 是语言配置的唯一来源。
    // Compiler 不硬编码每种语言的完整工具链细节，而是消费这里的 compiler_path、
    // compile_args、interpreter_path、run_args、extra_mounts 和 compile_limits。
    const LanguageRuntimeConfig& rt = LanguageManager::get_runtime(lang);

    // 1. 复制源码到 work_dir/<source_name>。
    //
    // 后续编译命令和解释器运行参数都以 work_dir 为当前目录，因此 compile_args
    // 和 run_args 可以使用相对路径，如 submission.cpp / Main.java / submission.py。
    const std::string dest = work_dir + "/" + rt.source_name;
    std::string copy_err;
    if (!copy_file(source_file, dest, copy_err)) {
        r.verdict = Verdict::SE;
        r.error_detail = copy_err;
        return r;
    }

    // 2. 解释型语言：无需编译。
    //
    // Python3 等语言在此直接返回解释器路径和脚本参数。真正执行用户程序仍由
    // Judge 层随后构造 SandboxRequest，并通过 Sandbox Core 运行。
    if (!rt.needs_compilation) {
        if (rt.interpreter_path.empty()) {
            r.verdict = Verdict::SE;
            r.error_detail = "interpreter not found for language '" + rt.name + "'";
            return r;
        }
        r.success = true;
        r.exec_path = rt.interpreter_path;
        r.exec_args = rt.run_args;
        return r;
    }

    // 3. 编译型语言：在沙箱内编译。
    //
    // Compiler 只负责构造编译请求，实际隔离执行由 Sandbox Core 负责。
    // req.is_compile=true 用于提示 Sandbox Core 当前是编译阶段：使用编译期资源限制，
    // 并在当前 Linux 后端中跳过运行期 seccomp 白名单。
    if (rt.compiler_path.empty()) {
        r.verdict = Verdict::SE;
        r.error_detail = "compiler not found for language '" + rt.name + "'";
        return r;
    }

    SandboxRequest req;
    req.executable = rt.compiler_path;
    req.argv = rt.compile_args;
    req.work_dir = work_dir;
    req.lang = rt.name;
    req.is_compile = true;
    req.limits = rt.compile_limits;
    req.envp = rt.compile_env;
    req.extra_mounts = rt.extra_mounts;
    req.stdout_path = work_dir + "/compile_stdout.txt";
    req.stderr_path = work_dir + "/compile_stderr.txt";

    // 4. 调用 Sandbox Core 执行编译器。
    //
    // 编译器 stdout/stderr 会落到 work_dir 中，Compiler 在执行后读回作为 CE 详情。
    SandboxResult sr = backend.execute(req);
    r.output = read_file(req.stdout_path) + read_file(req.stderr_path);
    r.exit_code = sr.exit_code;

    // Sandbox Core 自身失败不能伪装成 CE，必须作为 SE 向上层返回。
    if (sr.verdict == Verdict::SE) {
        r.verdict = Verdict::SE;
        r.error_detail = sr.error_detail;
        return r;
    }

    // 5. 编译成功判定：Sandbox 返回 AC + 编译器 exit_code=0 + artifact 存在。
    //
    // Java 这类“编译后仍由解释器/JVM 运行”的语言，会返回 interpreter_path；
    // C/C++/Go/Rust 这类 native 语言，运行阶段执行 work_dir 下的 ./artifact。
    const std::string artifact = work_dir + "/" + rt.artifact_name;
    const bool produced = file_exists(artifact);
    if (sr.verdict == Verdict::AC && sr.exit_code == 0 && produced) {
        r.success = true;
        if (rt.interpreter_path.empty()) {
            r.exec_path = "./" + rt.artifact_name;  // native，cwd=work_dir
            r.exec_args = {};
        } else {
            r.exec_path = rt.interpreter_path;       // 如 java
            r.exec_args = rt.run_args;
        }
        spdlog::debug("compiled {} -> {}", rt.name, rt.artifact_name);
    } else {
        r.success = false;
        r.verdict = Verdict::CE;
        if (r.output.empty()) {
            r.error_detail =
                "compiler did not produce artifact '" + rt.artifact_name +
                "'; sandbox verdict=" + verdict_to_string(sr.verdict) +
                ", exit_code=" + std::to_string(sr.exit_code) +
                ", signal=" + std::to_string(sr.signal_num);
        }
    }
    return r;
}

} // namespace cppjudge
