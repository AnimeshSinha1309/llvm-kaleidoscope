enable_testing()
set(SOURCE_FILES ../src/lexer.cpp ../src/ast.cpp ../src/parser.cpp)

add_executable(compiler_tests ${SOURCE_FILES} main.cpp test_lexer.cpp test_parser.cpp)
target_link_libraries(compiler_tests compiler_lib gtest)
add_test(compiler_tests compiler_tests . --gtest_output=xml:unit_tests_results/)
