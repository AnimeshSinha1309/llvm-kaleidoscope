#include <fstream>
#include <gtest/gtest.h>

#include "../src/lexer.hpp"

TEST(LexerTests, LexerOutputsSomeTokens)
{
    std::ifstream fin("../../test/test_simple.kld", std::ios::in);
    if (!fin.is_open())
        FAIL();
    auto token_list = kccani::tokenize(fin);
    ASSERT_GT(token_list.size(), 10);
}
