#include <fstream>
#include <iostream>
#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>

#include "../src/lexer.hpp"
#include "../src/parser.hpp"
#include "../src/evaluator.hpp"

using namespace kccani;

TEST(EvaluatorTests, SimpleArithmeticExpressionsGetsEvaluated)
{
    std::ifstream fin("../../test/sample_programs/test_expr.kld", std::ios::in);
    if (!fin.is_open())
        FAIL();

    kccani::Lexer lexer(fin);
    kccani::Parser parser = kccani::Parser(lexer);
    JITEvaluatorLLVM jit;

    auto ast_list = parser.fetch_all();
    auto llvm = std::visit(std::ref(jit), std::move(ast_list[0]));

    std::string actual_codegen = kccani::CodeGeneratorLLVM::to_string(std::move(llvm));
    auto result = jit.get();
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result.value(), 5.0);
}
