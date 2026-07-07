#pragma once

#include "cppjudge/common.h"
#include "cppjudge/sandbox.h"

#include <string>
#include <vector>

namespace cppjudge {

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
    // 用给定沙箱后端把提交编译（或复制）到 work_dir。
    // 编译型语言在沙箱内编译；解释型语言仅复制源码。
    static CompileResult compile(SandboxBackend& backend,
                                 const std::string& source_file,
                                 Language lang,
                                 const std::string& work_dir);
};

} // namespace cppjudge
