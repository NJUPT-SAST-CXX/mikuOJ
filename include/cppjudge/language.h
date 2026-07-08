#pragma once

#include "cppjudge/common.h"

#include <string>
#include <vector>

namespace cppjudge {

// seccomp 策略等级（平台无关标签；仅 Linux 后端据此选 syscall 白名单）。
enum class SeccompProfile { Strict, Standard, Extended, JVM };

// 一门语言的完整运行时配置。
//
// Language Manager 只负责描述“怎么编译 / 怎么运行 / 需要哪些依赖”；
// 它不直接执行编译器或用户程序。Compiler 和 Sandbox Core 消费这些配置：
//   - Compiler 使用 compiler_path、compile_args、compile_env、compile_limits；
//   - Sandbox Core 使用 seccomp_profile、extra_mounts 以及运行阶段 exec 信息。
//
//   运行模型：
//     - needs_compilation && interpreter_path 为空  → 原生：执行 work_dir/artifact_name
//     - interpreter_path 非空                        → 解释器/JVM：exec=interpreter_path, argv=run_args
struct LanguageRuntimeConfig {
    Language    lang = Language::UNKNOWN;
    std::string name;
    std::vector<std::string> extensions;
    bool        needs_compilation = false;

    std::string source_name;                 // 源码落地名（submission.cpp / Main.java / submission.py）
    std::string compiler_path;               // 运行时解析（解释型为空）
    std::vector<std::string> compile_args;   // 相对 work_dir，引用 source_name/artifact_name
    std::string artifact_name;               // 编译产物（native: "solution"；java: "Main.class"）
    std::vector<std::string> compile_env;    // 编译期追加环境（GO111MODULE 等）

    std::string interpreter_path;            // 解释器/JVM（native 为空）
    std::vector<std::string> run_args;       // 运行参数（python: {source_name}；java: {"-cp",".","Main"}）

    SeccompProfile seccomp_profile = SeccompProfile::Strict;   // 仅 Linux 后端使用
    std::vector<ns::MountEntry> extra_mounts;                  // 仅 Linux 后端使用
    Limits         compile_limits;                            // 编译阶段资源限制
};

class LanguageManager {
public:
    static Language detect_from_extension(const std::string& filename);
    static Language parse(const std::string& name);
    static const LanguageRuntimeConfig& get_runtime(Language lang);
    static const std::vector<Language>& supported_languages();
};

// 在 PATH 与常见目录中解析可执行文件的绝对路径（跨平台）；找不到返回 ""。
std::string resolve_tool(const std::vector<std::string>& candidates);

} // namespace cppjudge
