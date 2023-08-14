#include <fstream>
#include <iostream>
#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

#include "../src/lexer.hpp"
#include "../src/parser.hpp"
#include "../src/codegen.hpp"

using namespace kccani;

TEST(CodegenTests, CorrectLlvmIrGeneratedForASimpleFunctionDefn)
{
    std::ifstream fin("../../test/sample_programs/test_arithmetic.kld", std::ios::in);
    if (!fin.is_open())
        FAIL();

    kccani::Lexer lexer(fin);
    kccani::Parser parser = kccani::Parser(lexer);
    CodeGeneratorLLVM codegen;

    auto ast_list = parser.fetch_all();
    auto llvm = std::visit(std::ref(codegen), std::move(ast_list[0]));
    std::string actual_codegen = kccani::CodeGeneratorLLVM::to_string(std::move(llvm));

    const std::string expected_codegen =
        "define double @test(double \%x) {\n"
        "entry:\n"
        "  \%addtmp = fadd double 3.000000e+00, \%x\n"
        "  \%addtmp1 = fadd double \%x, 3.000000e+00\n"
        "  %multmp = fmul double \%addtmp, \%addtmp1\n"
        "  ret double %multmp\n"
        "}\n";
    ASSERT_EQ(actual_codegen, expected_codegen);
}
