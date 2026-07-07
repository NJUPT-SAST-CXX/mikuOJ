#include "cppjudge/doctor.h"

#include "cppjudge/language.h"

#include <unistd.h>

#include <iostream>
#include <string>

#if defined(__linux__)
#include <sys/utsname.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#endif

namespace cppjudge {

namespace {

struct Check {
    const char* name;
    bool        ok;
    bool        hard;   // 硬检查失败 → NOT_READY
    std::string hint;
};

void print_check(const Check& c) {
    const char* tag = c.ok ? "PASS" : (c.hard ? "FAIL" : "WARN");
    std::cout << "  [" << tag << "] " << c.name;
    if (!c.ok && !c.hint.empty()) std::cout << "\n         -> " << c.hint;
    std::cout << "\n";
}

} // namespace

bool Doctor::check() {
    std::cout << "cppjudge environment check\n"
              << "==========================\n\n";
    bool hard_ok = true;
    auto run = [&](Check c) {
        print_check(c);
        if (c.hard && !c.ok) hard_ok = false;
    };

#if defined(__linux__)
    struct utsname u{};
    const bool have_uname = uname(&u) == 0;
    const std::string release = have_uname ? u.release : "";

    const bool is_wsl = release.find("microsoft") != std::string::npos ||
                        release.find("WSL") != std::string::npos;
    run({"WSL2 kernel (advisory)", is_wsl, false,
         "expected on WSL2 dev; ignore on native Linux/CI"});

    run({"running as root", geteuid() == 0, true,
         "run with sudo (namespaces/cgroups require privilege)"});

    int major = 0, minor = 0;
    if (have_uname) std::sscanf(release.c_str(), "%d.%d", &major, &minor);
    run({"kernel >= 5.15", (major > 5) || (major == 5 && minor >= 15), true,
         "update kernel / WSL2"});

    bool cgv2 = false;
    {
        std::ifstream mounts("/proc/mounts");
        std::string line;
        while (std::getline(mounts, line)) {
            if (line.find("cgroup2") != std::string::npos) { cgv2 = true; break; }
        }
    }
    run({"cgroup v2", cgv2, true, "mount cgroup2 at /sys/fs/cgroup"});

    const bool seccomp =
        std::ifstream("/usr/include/seccomp.h").good() ||
        std::system("ldconfig -p 2>/dev/null | grep -q libseccomp") == 0;
    run({"libseccomp", seccomp, true, "apt install libseccomp2 libseccomp-dev"});
#elif defined(__APPLE__)
    std::cout << "  [WARN] macOS development build: NO sandbox isolation\n"
              << "         -> seccomp / cgroup / namespaces are Linux-only.\n"
              << "         -> builds here are for development and pipeline testing only;\n"
              << "            never judge untrusted submissions on macOS.\n";
#endif

    // 工具链（两平台通用）
    run({"C++ compiler", !resolve_tool({"g++", "c++", "clang++"}).empty(), false,
         "install a C++20 compiler"});
    run({"python3", !resolve_tool({"python3", "python"}).empty(), false,
         "install python3"});

    std::cout << "\n";
    if (hard_ok) {
        std::cout << "READY.\n";
    } else {
        std::cout << "NOT_READY (hard checks failed).\n";
    }
    return hard_ok;
}

} // namespace cppjudge
