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
    auto token_list = kccani::tokenize(fin);
    auto ast_list = parse_program(token_list);
    CodeGeneratorLLVM codegen;

    for (const auto &ast : ast_list)
    {
        
    }
}