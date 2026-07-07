#pragma once

#include "cppjudge/common.h"
#include "cppjudge/sandbox.h"

#include <string>
#include <vector>

namespace cppjudge {

// Compiler 对外输出：一次编译阶段的结果。
//
// Compiler 位于 Judge/CLI 与 Sandbox Core 之间：
//   - 上层传入 source_file、Language 和 work_dir；
//   - Compiler 查询 Language Manager，决定是否需要编译；
//   - 对编译型语言，Compiler 构造 SandboxRequest 并交给 Sandbox Core 执行；
//   - 对解释型语言，Compiler 只复制源码并返回解释器执行信息。
//
// 注意：CompileResult 不负责比较输出，也不代表测试点运行结果。
struct CompileResult {
    bool        success = false;
    std::string output;                  // 编译器 stdout+stderr（CE 详情）
    std::string exec_path;               // 运行阶段可执行文件（native: "./solution"；解释器/JVM 绝对路径）
    std::vector<std::string> exec_args;  // exec_path 之后的参数
    int         exit_code = 0;
    Verdict     verdict = Verdict::AC;    // 失败时为 CE 或 SE
    std::string error_detail;            // SE 详情
};

class Compiler {
public:
    // 对外课程/任务接口：自动选择默认安全沙箱后端完成编译或源码复制。
    static CompileResult compile(const std::string& source_file,
                                 Language lang,
                                 const std::string& work_dir);

    // 用给定沙箱后端把提交准备到 work_dir。
    //
    // 编译型语言：
    //   1. 复制源码到 work_dir/<source_name>；
    //   2. 根据 LanguageRuntimeConfig 构造编译命令；
    //   3. 通过 SandboxBackend 在沙箱内执行编译器；
    //   4. 检查退出码和 artifact 是否生成；
    //   5. 返回运行阶段需要的 exec_path / exec_args。
    //
    // 解释型语言：
    //   1. 复制源码到 work_dir/<source_name>；
    //   2. 跳过编译；
    //   3. 返回解释器路径和脚本参数。
    static CompileResult compile(SandboxBackend& backend,
                                 const std::string& source_file,
                                 Language lang,
                                 const std::string& work_dir);

    // 仅复制源文件到工作目录，用于解释型语言或单元测试。
    static CompileResult copy_source(const std::string& source_file,
                                     Language lang,
                                     const std::string& work_dir);
};

} // namespace cppjudge
