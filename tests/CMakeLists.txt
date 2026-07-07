# ============================================================
# 测试标签约定（供 CI ctest -L 过滤）
#   -L unit         → 单元测试（Google Test）
#   -L integration  → 集成测试（shell 脚本）
#   -L security     → 安全测试（手动运行）
# ============================================================

find_package(GTest REQUIRED)
include(GoogleTest)

# ---- 平台无关模块单元测试（macOS + Linux 都能跑，无需 root）----
set(CPPJUDGE_UNIT_TESTS
    comparator
    problem
    language
    logger
)

# ---- Linux 隔离模块单元测试（Phase B，仅 Linux）----
if(UNIX AND NOT APPLE)
    list(APPEND CPPJUDGE_UNIT_TESTS
        seccomp_manager
        cgroup_manager
        ns_manager
    )
endif()

foreach(m ${CPPJUDGE_UNIT_TESTS})
    add_executable(test_${m} unit/test_${m}.cpp)
    target_link_libraries(test_${m} PRIVATE cppjudge_core GTest::gtest GTest::gtest_main)
    gtest_discover_tests(test_${m}
        PROPERTIES LABELS unit
        # 让 "problems/A+B"、"build/..." 等相对路径以源码根为基准
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach()

# 集成测试（9 判决）与安全测试在 Phase C 接入 add_subdirectory(integration)
