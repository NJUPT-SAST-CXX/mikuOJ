# 集成测试（9 判决 + 多语言 + 浮点）。需要 cppjudge 二进制；Linux 上用安全后端。
set(INTEGRATION_TESTS ac wa ce tle mle ole re sv se floating all_languages)

foreach(t ${INTEGRATION_TESTS})
    add_test(NAME integration_${t}
             COMMAND ${CMAKE_SOURCE_DIR}/tests/integration/test_${t}.sh
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(integration_${t} PROPERTIES
        LABELS integration
        ENVIRONMENT "CPPJUDGE=$<TARGET_FILE:cppjudge>")
endforeach()

# 安全测试（手动/单独触发：ctest -L security）
add_test(NAME security_suite
         COMMAND ${CMAKE_SOURCE_DIR}/tests/security/run_security_tests.sh
         WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_tests_properties(security_suite PROPERTIES
    LABELS security
    ENVIRONMENT "CPPJUDGE=$<TARGET_FILE:cppjudge>")
