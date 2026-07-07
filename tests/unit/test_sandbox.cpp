#include <gtest/gtest.h>

#include "cppjudge/sandbox.h"

#include <filesystem>

using namespace cppjudge;
namespace fs = std::filesystem;

namespace {

fs::path make_temp_dir(const std::string& name) {
    fs::path dir = fs::temp_directory_path() / name;
    fs::create_directories(dir);
    return dir;
}

}  // namespace

TEST(SandboxApi, ExposesTaskInterfaceNames) {
    SandboxConfig config;
    config.lang = "cpp";
    config.work_dir = ".";

    SandboxRequest request = config;  // 内部别名仍可用
    EXPECT_EQ(request.lang, "cpp");
}

TEST(SandboxApi, ExecuteInvalidProgramReturnsFailureResult) {
    fs::path dir = make_temp_dir("mikuoj-test-sandbox-invalid");

    SandboxConfig config;
    config.executable = "/definitely/not/a/real/program";
    config.work_dir = dir.string();
    config.lang = "cpp";
    config.limits.cpu_time_ms = 1000;
    config.limits.wall_time_ms = 3000;
    config.limits.memory_mb = 64;

    SandboxResult result = Sandbox::execute(config);
    EXPECT_NE(result.verdict, Verdict::AC);
}

