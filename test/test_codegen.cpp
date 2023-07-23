#include <fstream>
#include <iostream>
#include <gtest/gtest.h>

#include "../src/lexer.hpp"
#include "../src/parser.hpp"
#include "../src/codegen.hpp"

using namespace kccani;

TEST(CodegenTests, LlvmIrIsGeneratedFromASimpleFunctionDefnAndCall)
{
    std::ifstream fin("../../test/sample_programs/test_simple.kld", std::ios::in);
    if (!fin.is_open())
        FAIL();
    kccani::Lexer lexer(fin);
    auto parser = kccani::Parser(lexer);
    auto ast_list = parser.fetch_all();
    CodeGeneratorLLVM codegen;

    for (const auto &ast : ast_list)
    {
        
    }
}