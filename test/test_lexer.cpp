#include <fstream>
#include <iostream>
#include <gtest/gtest.h>

#include "../src/lexer.hpp"

using namespace kccani;

TEST(LexerTests, OutputsTheRightNumberOfTokensOfCorrectTypes)
{
    std::ifstream fin("../../test/sample_programs/test_simple.kld", std::ios::in);
    if (!fin.is_open())
        FAIL();
    auto token_list = kccani::tokenize(fin);
    
    int count_def = 0, count_var = 0, count_val = 0, count_op = 0, count_eof;
    for (auto token : token_list)
    {
        switch (token.type)
        {
        case Token::TokenType::TOKEN_DEF:
            count_def++;
            break;
        case Token::TokenType::TOKEN_EOF:
            count_eof++;
            break;
        case Token::TokenType::TOKEN_IDENTIFIER:
            count_var++;
            break;
        case Token::TokenType::TOKEN_NUMBER:
            count_val++;
            break;
        case Token::TokenType::TOKEN_SPECIAL:
            count_op++;
            break;
        }
    }
    EXPECT_EQ(count_def, 1);
    EXPECT_EQ(count_eof, 1);
    EXPECT_EQ(count_var, 7);
    EXPECT_EQ(count_val, 3);
    EXPECT_EQ(count_op, 12);
}
