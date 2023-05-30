enable_testing()

add_executable(compiler_tests main.cpp test_lexer.cpp)
target_link_libraries(compiler_tests compiler_lib gtest)
add_test(compiler_tests compiler_tests . --gtest_output=xml:unit_tests_results/)
