#include <gtest/gtest.h>

#include "cppjudge/compiler.h"
#include "cppjudge/language.h"

#include <filesystem>
#include <fstream>

#if defined(__linux__)
#include <unistd.h>
#endif

using namespace cppjudge;
namespace fs = std::filesystem;

namespace {

fs::path make_temp_dir(const std::string& name) {
    fs::path dir = fs::temp_directory_path() / name;
    fs::create_directories(dir);
    return dir;
}

fs::path write_file(const fs::path& path, const std::string& content) {
    std::ofstream f(path, std::ios::binary);
    f << content;
    return path;
}

}  // namespace

TEST(CompilerApi, CopySourceForPython) {
    fs::path dir = make_temp_dir("mikuoj-test-compiler-copy-python");
    fs::path src = write_file(dir / "input.py", "print(1 + 2)\n");

    CompileResult result = Compiler::copy_source(src.string(), Language::PYTHON3, dir.string());

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_FALSE(result.exec_path.empty());
    ASSERT_FALSE(result.exec_args.empty());
    EXPECT_EQ(result.exec_args.front(), "submission.py");
    EXPECT_TRUE(fs::exists(dir / "submission.py"));
}

TEST(CompilerApi, PublicCompileSkipsCompilationForPython) {
    fs::path dir = make_temp_dir("mikuoj-test-compiler-public-python");
    fs::path src = write_file(dir / "input.py", "print('ok')\n");

    CompileResult result = Compiler::compile(src.string(), Language::PYTHON3, dir.string());

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.verdict, Verdict::AC);
    EXPECT_FALSE(result.exec_path.empty());
    EXPECT_TRUE(fs::exists(dir / "submission.py"));
}

TEST(CompilerApi, CompileCppViaSandboxWhenPrivileged) {
#if defined(__linux__)
    if (geteuid() != 0) {
        GTEST_SKIP() << "needs root for namespace/cgroup sandbox";
    }
    const auto& rt = LanguageManager::get_runtime(Language::CPP);
    if (rt.compiler_path.empty()) {
        GTEST_SKIP() << "C++ compiler not available";
    }

    fs::path dir = make_temp_dir("mikuoj-test-compiler-cpp");
    fs::path src = write_file(dir / "main.cpp",
                              "#include <iostream>\n"
                              "int main(){ std::cout << 3 << '\\n'; }\n");

    CompileResult result = Compiler::compile(src.string(), Language::CPP, dir.string());

    EXPECT_TRUE(result.success) << result.output << result.error_detail;
    EXPECT_EQ(result.exit_code, 0);
    EXPECT_FALSE(result.exec_path.empty());
    EXPECT_TRUE(fs::exists(dir / "solution"));
#else
    GTEST_SKIP() << "linux sandbox only";
#endif
}

