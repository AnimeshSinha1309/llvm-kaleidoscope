enable_testing()

add_executable(compiler_tests main.cpp test_lexer.cpp test_parser.cpp test_codegen.cpp)
target_link_libraries(compiler_tests compiler_lib gtest gmock)
add_test(
    NAME compiler_tests
    COMMAND compiler_tests . --gtest_output=xml:unit_tests_results/
    WORKING_DIRECTORY .
)
