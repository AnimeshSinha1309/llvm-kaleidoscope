#include <memory>
#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/ast.hpp"
#include "../src/lexer.hpp"
#include "../src/parser.hpp"

namespace kccani 
{

class MockLexer : public Lexer
{
public:
    MockLexer(std::istream& stream) : Lexer(stream) {};
    MOCK_METHOD(Token, get, (), (override));
    MOCK_METHOD(Token, peek, (), (override));
};


TEST(ParserTests, NumberExpressionsGetParsedAsNumberExpr)
{
    // 3
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
        Token{Token::TokenType::TOKEN_EOF},
    };
    auto lexer = MockLexer(std::cin);
    ON_CALL(lexer, get()).WillByDefault([&token_list]() -> Token {
        auto value = token_list.front();
        token_list.pop_front();
        return value;
    });
    ON_CALL(lexer, peek()).WillByDefault([&token_list]() -> Token {
        return token_list.front();
    });

    auto parser = Parser(lexer);
    std::unique_ptr<ExprAST> expr = parser.parse_number_expr();

    NumberExprAST* ast(dynamic_cast<NumberExprAST*>(expr.get()));
    ASSERT_EQ(ast->value, 3.0);
}

TEST(ParserTests, NumberExpressionsGetParsedAsPrimaryExpr)
{
    // 3
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_NUMBER, 3.0},
        Token{Token::TokenType::TOKEN_EOF},
    };
    auto lexer = MockLexer(std::cin);
    ON_CALL(lexer, get()).WillByDefault([&token_list]() -> Token {
        auto value = token_list.front();
        token_list.pop_front();
        return value;
    });
    ON_CALL(lexer, peek()).WillByDefault([&token_list]() -> Token {
        return token_list.front();
    });

    auto parser = Parser(lexer);
    std::unique_ptr<ExprAST> expr = parser.parse_primary();

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
        Token{Token::TokenType::TOKEN_EOF},
    };
    auto lexer = MockLexer(std::cin);
    ON_CALL(lexer, get()).WillByDefault([&token_list]() -> Token {
        auto value = token_list.front();
        token_list.pop_front();
        return value;
    });
    ON_CALL(lexer, peek()).WillByDefault([&token_list]() -> Token {
        return token_list.front();
    });

    auto parser = Parser(lexer);
    std::unique_ptr<ExprAST> expr = parser.parse_expr();

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
        Token{Token::TokenType::TOKEN_EOF},
    };
    auto lexer = MockLexer(std::cin);
    ON_CALL(lexer, get()).WillByDefault([&token_list]() -> Token {
        auto value = token_list.front();
        token_list.pop_front();
        return value;
    });
    ON_CALL(lexer, peek()).WillByDefault([&token_list]() -> Token {
        return token_list.front();
    });

    auto parser = Parser(lexer);
    std::unique_ptr<ExprAST> expr = parser.parse_expr();

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
        Token{Token::TokenType::TOKEN_EOF},
    };
    auto lexer = MockLexer(std::cin);
    ON_CALL(lexer, get()).WillByDefault([&token_list]() -> Token {
        auto value = token_list.front();
        token_list.pop_front();
        return value;
    });
    ON_CALL(lexer, peek()).WillByDefault([&token_list]() -> Token {
        return token_list.front();
    });

    auto parser = Parser(lexer);
    std::unique_ptr<ExprAST> expr = parser.parse_expr();

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
        Token{Token::TokenType::TOKEN_EOF},
    };
    auto lexer = MockLexer(std::cin);
    ON_CALL(lexer, get()).WillByDefault([&token_list]() -> Token {
        auto value = token_list.front();
        token_list.pop_front();
        return value;
    });
    ON_CALL(lexer, peek()).WillByDefault([&token_list]() -> Token {
        return token_list.front();
    });

    auto parser = Parser(lexer);
    std::unique_ptr<ExprAST> expr = parser.parse_primary();

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
        Token{Token::TokenType::TOKEN_EOF},
    };
    auto lexer = MockLexer(std::cin);
    ON_CALL(lexer, get()).WillByDefault([&token_list]() -> Token {
        auto value = token_list.front();
        token_list.pop_front();
        return value;
    });
    ON_CALL(lexer, peek()).WillByDefault([&token_list]() -> Token {
        return token_list.front();
    });

    auto parser = Parser(lexer);
    std::unique_ptr<ExprAST> expr = parser.parse_expr();
    BinaryExprAST* ast(dynamic_cast<BinaryExprAST*>(expr.get()));
    ASSERT_EQ(
        expr->to_string(),
        "(3.000000) * ((2.000000) + ((5.000000) * (4.000000)))");
}

TEST(ParserTests, ParsesFunctionPrototypeCorrectly)
{
    // 3 * (2 + 5 * 4)
    std::deque<Token> token_list = {
        Token{Token::TokenType::TOKEN_IDENTIFIER, "func"},
        Token{Token::TokenType::TOKEN_SPECIAL, '('},
        Token{Token::TokenType::TOKEN_IDENTIFIER, "x"},
        Token{Token::TokenType::TOKEN_IDENTIFIER, "y"},
        Token{Token::TokenType::TOKEN_SPECIAL, ')'},
        Token{Token::TokenType::TOKEN_EOF},
    };
    auto lexer = MockLexer(std::cin);
    ON_CALL(lexer, get()).WillByDefault([&token_list]() -> Token {
        auto value = token_list.front();
        token_list.pop_front();
        return value;
    });
    ON_CALL(lexer, peek()).WillByDefault([&token_list]() -> Token {
        return token_list.front();
    });

    auto parser = Parser(lexer);
    std::unique_ptr<FunctionPrototypeAST> fn = parser.parse_function_proto();

    FunctionPrototypeAST* ast(dynamic_cast<FunctionPrototypeAST*>(fn.get()));
    ASSERT_EQ(ast->to_string(), "def func(x, y)");
}

TEST(ParserTests, TopLevelParsingParsesASimpleFileWithFunctionDefinitions)
{
    std::ifstream fin("../../test/sample_programs/test_simple.kld", std::ios::in);
    if (!fin.is_open())
        FAIL();
    auto lexer = Lexer(fin);
    auto parser = Parser(lexer);
    auto ast_list = parser.fetch_all();

    ASSERT_EQ(ast_list.size(), 2);

    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<FunctionAST>>(ast_list[0]));
    auto ast_1 = std::get<std::unique_ptr<FunctionAST>>(std::move(ast_list[0]));
    ASSERT_EQ(ast_1->to_string(), "def fib(x){(fib((x) - (1.000000))) + (fib((x) - (2.000000)))}");

    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<ExprAST>>(ast_list[1]));
    auto ast_2 = std::get<std::unique_ptr<ExprAST>>(std::move(ast_list[1]));
    ASSERT_EQ(ast_2->to_string(), "fib(6.000000)");
}

TEST(ParserTests, TopLevelParsingParsesASimpleFileWithExternAndCall)
{
    std::ifstream fin("../../test/sample_programs/test_extern.kld", std::ios::in);
    if (!fin.is_open())
        FAIL();
    auto lexer = Lexer(fin);
    auto parser = Parser(lexer);
    auto ast_list = parser.fetch_all();

    ASSERT_EQ(ast_list.size(), 2);

    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<FunctionPrototypeAST>>(ast_list[0]));
    auto ast_1 = std::get<std::unique_ptr<FunctionPrototypeAST>>(std::move(ast_list[0]));
    ASSERT_EQ(ast_1->to_string(), "def atan2(x, y)");

    ASSERT_TRUE(std::holds_alternative<std::unique_ptr<ExprAST>>(ast_list[1]));
    auto ast_2 = std::get<std::unique_ptr<ExprAST>>(std::move(ast_list[1]));
    ASSERT_EQ(ast_2->to_string(), "atan2(13.000000, (5.000000) + (8.000000))");
}

}
