set(SOURCE_FILES lexer.cpp ast.cpp parser.cpp codegen.cpp)

add_library(compiler_lib ${SOURCE_FILES})
target_link_libraries(compiler_lib Boost::program_options spdlog::spdlog
                      ${LLVM_LIBS})

add_executable(compiler compiler.cpp)
target_link_libraries(compiler compiler_lib Boost::program_options
                      spdlog::spdlog)
