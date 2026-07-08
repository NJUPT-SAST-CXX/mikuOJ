#include <gtest/gtest.h>

#include "cppjudge/language.h"
#include "cppjudge/sandbox.h"

#include <filesystem>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#if defined(__linux__)
#include <unistd.h>
#endif

using namespace cppjudge;
namespace fs = std::filesystem;

namespace {

bool privileged() {
#if defined(__linux__)
    return geteuid() == 0;
#else
    return false;
#endif
}

fs::path make_temp_dir(const std::string& name) {
    fs::path dir = fs::temp_directory_path() / name;
    fs::create_directories(dir);
    return dir;
}

fs::path compile_c(const fs::path& dir, const std::string& name,
                   const std::string& code) {
    fs::path src = dir / (name + ".c");
    fs::path bin = dir / name;
    { std::ofstream f(src); f << code; }
    std::string cmd = "gcc -std=c11 -O0 -static -o " + bin.string()
                      + " " + src.string() + " 2>/dev/null";
    std::system(cmd.c_str());
    return bin;
}

std::string read_file(const fs::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
}

// executable 用 ./name 而非绝对路径：沙箱 pivot_root 后宿主机路径不可见
SandboxConfig make_cfg(const fs::path& dir, const std::string& bin_name) {
    SandboxConfig c;
    c.executable = "./" + bin_name;
    c.work_dir = dir.string();
    c.lang = "c";
    c.limits = {2000, 6000, 64, 8, 10, 64, 5000};
    c.extra_mounts = LanguageManager::get_runtime(Language::C).extra_mounts;
    c.extra_mounts.push_back({dir.string(), dir.string(), true});
    return c;
}

}  // namespace

// ---- 脚手架原有 ----
TEST(SandboxApi, ExposesTaskInterfaceNames) {
    SandboxConfig c; c.lang = "cpp"; c.work_dir = ".";
    SandboxRequest r = c;
    EXPECT_EQ(r.lang, "cpp");
}

TEST(SandboxApi, ExecuteInvalidProgramReturnsFailureResult) {
    fs::path dir = make_temp_dir("mikuoj-test-sandbox-invalid");
    SandboxConfig c;
    c.executable = "/definitely/not/a/real/program";
    c.work_dir = dir.string();
    c.lang = "cpp";
    c.limits = {1000, 3000, 64};
    EXPECT_NE(Sandbox::execute(c).verdict, Verdict::AC);
}

// ---- 新增：执行成功 + 输出捕获 ----
TEST(Sandbox, ExecuteEchoCapturesStdout) {
    if (!privileged()) GTEST_SKIP() << "needs root";
    auto dir = make_temp_dir("mikuoj-test-sandbox-stdout");
    ASSERT_TRUE(fs::exists(compile_c(dir, "echo_prog",
        "#include <stdio.h>\n"
        "int main(){printf(\"hello sandbox\\n\");return 0;}\n")));

    auto out = dir / "stdout.txt";
    auto cfg = make_cfg(dir, "echo_prog");
    cfg.stdout_path = out.string();

    auto r = Sandbox::execute(cfg);
    EXPECT_EQ(r.verdict, Verdict::AC);
    EXPECT_EQ(r.exit_code, 0);
    EXPECT_EQ(read_file(out), "hello sandbox\n");
}

// ---- 新增：非零退出 → RE ----
TEST(Sandbox, NonZeroExitCodeMapsToRuntimeError) {
    if (!privileged()) GTEST_SKIP() << "needs root";
    auto dir = make_temp_dir("mikuoj-test-sandbox-re");
    ASSERT_TRUE(fs::exists(compile_c(dir, "exit_42",
        "int main(){return 42;}\n")));

    auto r = Sandbox::execute(make_cfg(dir, "exit_42"));
    EXPECT_EQ(r.verdict, Verdict::RE);
    EXPECT_EQ(r.exit_code, 42);
}

// ---- 新增：超时 → TLE ----
TEST(Sandbox, InfiniteLoopTriggersTLE) {
    if (!privileged()) GTEST_SKIP() << "needs root";
    auto dir = make_temp_dir("mikuoj-test-sandbox-tle");
    ASSERT_TRUE(fs::exists(compile_c(dir, "spin",
        "int main(){while(1){}}\n")));

    auto cfg = make_cfg(dir, "spin");
    cfg.limits.cpu_time_ms = 500;
    cfg.limits.wall_time_ms = 1500;

    EXPECT_EQ(Sandbox::execute(cfg).verdict, Verdict::TLE);
}

// ---- 新增：fork 在 Strict 白名单外 → SV ----
TEST(Sandbox, ForkInStrictProfileTriggersSV) {
    if (!privileged()) GTEST_SKIP() << "needs root";
    auto dir = make_temp_dir("mikuoj-test-sandbox-sv");
    ASSERT_TRUE(fs::exists(compile_c(dir, "forker",
        "#include <unistd.h>\n#include <sys/wait.h>\n"
        "int main(){int s;fork();wait(&s);return 0;}\n")));

    auto r = Sandbox::execute(make_cfg(dir, "forker"));
    EXPECT_EQ(r.verdict, Verdict::SV)
        << "fork() must be blocked by seccomp Strict profile";
}

// ---- 新增：exit 0 → AC ----
TEST(Sandbox, NormalExitWithCodeZeroReturnsAC) {
    if (!privileged()) GTEST_SKIP() << "needs root";
    auto dir = make_temp_dir("mikuoj-test-sandbox-ac");
    ASSERT_TRUE(fs::exists(compile_c(dir, "ok",
        "int main(){return 0;}\n")));

    auto r = Sandbox::execute(make_cfg(dir, "ok"));
    EXPECT_EQ(r.verdict, Verdict::AC);
    EXPECT_EQ(r.exit_code, 0);
}

// ---- 新增：资源统计非零 ----
TEST(Sandbox, ResourceMetricsAreCollected) {
    if (!privileged()) GTEST_SKIP() << "needs root";
    auto dir = make_temp_dir("mikuoj-test-sandbox-metrics");
    ASSERT_TRUE(fs::exists(compile_c(dir, "busy",
        "#include <stdlib.h>\n#include <string.h>\n"
        "int main(){"
        "volatile char*p=malloc(1<<20);"
        "memset((void*)p,0,1<<20);"
        "for(volatile int i=0;i<1000000;i++){}"
        "free((void*)p);return 0;}\n")));

    auto cfg = make_cfg(dir, "busy");
    cfg.limits.memory_mb = 128;

    auto r = Sandbox::execute(cfg);
    EXPECT_EQ(r.verdict, Verdict::AC);
    EXPECT_GT(r.time_ms, 0u);
    EXPECT_GT(r.memory_kb, 0u);
}

// ---- 新增：内存炸弹 → MLE ----
TEST(Sandbox, MemoryBombTriggersMLE) {
    if (!privileged()) GTEST_SKIP() << "needs root";
    auto dir = make_temp_dir("mikuoj-test-sandbox-mle");
    ASSERT_TRUE(fs::exists(compile_c(dir, "hog",
        "#include <stdlib.h>\n#include <string.h>\n"
        "int main(){"
        "while(1){volatile char*p=malloc(10<<20);if(p)memset((void*)p,0,10<<20);}"
        "return 0;}\n")));

    auto cfg = make_cfg(dir, "hog");
    cfg.limits.memory_mb = 32;

    auto r = Sandbox::execute(cfg);
    EXPECT_EQ(r.verdict, Verdict::MLE);
}
