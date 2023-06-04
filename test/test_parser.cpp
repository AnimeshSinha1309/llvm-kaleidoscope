#include <memory>
#include <iostream>
#include <gtest/gtest.h>

#include "../src/ast.hpp"
#include "../src/parser.hpp"

using namespace kccani;

TEST(ParserTests, NumberExpressionsGetParsedAsNumberExpr)
{
    // 3
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
    };
    std::unique_ptr<ExprAST> expr = parse_number_expr(token_list);
    NumberExprAST* ast(dynamic_cast<NumberExprAST*>(expr.get()));
    ASSERT_EQ(ast->value, 3.0);
}

TEST(ParserTests, NumberExpressionsGetParsedAsPrimaryExpr)
{
    // 3
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
    };
    std::unique_ptr<ExprAST> expr = parse_primary(token_list);
    NumberExprAST* ast(dynamic_cast<NumberExprAST*>(expr.get()));
    ASSERT_EQ(ast->value, 3.0);
}

TEST(ParserTests, SimpleBinaryExpressionsGetParsed)
{
    // 3 + 2
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '+'},
        Token{Token::TokenType::TOKEN_NUMBER, 2.0},
    };
    std::unique_ptr<ExprAST> expr = parse_expr(token_list);
    BinaryExprAST* ast = dynamic_cast<BinaryExprAST*>(expr.get());
    ASSERT_EQ(ast->opcode, '+');
    NumberExprAST* lhs = dynamic_cast<NumberExprAST*>(ast->lhs.get());
    ASSERT_EQ(lhs->value, 3.0);
    NumberExprAST* rhs = dynamic_cast<NumberExprAST*>(ast->rhs.get());
    ASSERT_EQ(rhs->value, 2.0);
}

TEST(ParserTests, BinaryOperatorExpressionsGetParsedWithCorrectPrecedence1)
{
    // 3 + 2 * 5
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '+'},
        Token{Token::TokenType::TOKEN_NUMBER, 2.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '*'},
        Token{Token::TokenType::TOKEN_NUMBER, 5.0},
    };
    std::unique_ptr<ExprAST> expr = parse_expr(token_list);
    BinaryExprAST* ast = dynamic_cast<BinaryExprAST*>(expr.get());
    ASSERT_EQ(ast->opcode, '+');
    NumberExprAST* lhs = dynamic_cast<NumberExprAST*>(ast->lhs.get());
    ASSERT_EQ(lhs->value, 3.0);
    BinaryExprAST* rhs = dynamic_cast<BinaryExprAST*>(ast->rhs.get());
    NumberExprAST* rlhs = dynamic_cast<NumberExprAST*>(rhs->lhs.get());
    NumberExprAST* rrhs = dynamic_cast<NumberExprAST*>(rhs->rhs.get());
    ASSERT_EQ(rhs->opcode, '*');
    ASSERT_EQ(rlhs->value, 2.0);
    ASSERT_EQ(rrhs->value, 5.0);
}

TEST(ParserTests, BinaryOperatorExpressionsGetParsedWithCorrectPrecedence2)
{
    // 3 * 2 + 5
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '*'},
        Token{Token::TokenType::TOKEN_NUMBER, 2.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '+'},
        Token{Token::TokenType::TOKEN_NUMBER, 5.0},
    };
    std::unique_ptr<ExprAST> expr = parse_expr(token_list);
    BinaryExprAST* ast = dynamic_cast<BinaryExprAST*>(expr.get());
    ASSERT_EQ(ast->opcode, '+');
    BinaryExprAST* lhs = dynamic_cast<BinaryExprAST*>(ast->lhs.get());
    NumberExprAST* llhs = dynamic_cast<NumberExprAST*>(lhs->lhs.get());
    NumberExprAST* lrhs = dynamic_cast<NumberExprAST*>(lhs->rhs.get());
    ASSERT_EQ(lhs->opcode, '*');
    ASSERT_EQ(llhs->value, 3.0);
    ASSERT_EQ(lrhs->value, 2.0);
    NumberExprAST* rhs = dynamic_cast<NumberExprAST*>(ast->rhs.get());
    ASSERT_EQ(rhs->value, 5.0);
}

TEST(ParserTests, BracketedBinaryExpressionsGetParsed)
{
    // (3 + 2)
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_SPECIAL, '('},
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '+'},
        Token{Token::TokenType::TOKEN_NUMBER, 2.0},
        Token{Token::TokenType::TOKEN_SPECIAL, ')'},
    };
    std::unique_ptr<ExprAST> expr = parse_primary(token_list);
    BinaryExprAST* ast = dynamic_cast<BinaryExprAST*>(expr.get());
    ASSERT_EQ(ast->opcode, '+');
    NumberExprAST* lhs = dynamic_cast<NumberExprAST*>(ast->lhs.get());
    ASSERT_EQ(lhs->value, 3.0);
    NumberExprAST* rhs = dynamic_cast<NumberExprAST*>(ast->rhs.get());
    ASSERT_EQ(rhs->value, 2.0);
}

TEST(ParserTests, GeneratesTheCorrectParsedExpression)
{
    // 3 * (2 + 5 * 4)
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '*'},
        Token{Token::TokenType::TOKEN_SPECIAL, '('},
        Token{Token::TokenType::TOKEN_NUMBER, 2.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '+'},
        Token{Token::TokenType::TOKEN_NUMBER, 5.0},
        Token{Token::TokenType::TOKEN_SPECIAL, '*'},
        Token{Token::TokenType::TOKEN_NUMBER, 4.0},
        Token{Token::TokenType::TOKEN_SPECIAL, ')'},
    };
    std::unique_ptr<ExprAST> expr = parse_expr(token_list);
    BinaryExprAST* ast(dynamic_cast<BinaryExprAST*>(expr.get()));
    ASSERT_EQ(
        expr->to_string(),
        "(3.000000) * ((2.000000) + ((5.000000) * (4.000000)))");
}
