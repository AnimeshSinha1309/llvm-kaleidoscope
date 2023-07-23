#include <deque>
#include <memory>
#include <gtest/gtest.h>

#include "ast.hpp"
#include "lexer.hpp"

namespace kccani
{

class Parser
{
private:

    FRIEND_TEST(ParserTests, NumberExpressionsGetParsedAsNumberExpr);
    FRIEND_TEST(ParserTests, NumberExpressionsGetParsedAsPrimaryExpr);
    FRIEND_TEST(ParserTests, SimpleBinaryExpressionsGetParsed);
    FRIEND_TEST(ParserTests, BinaryOperatorExpressionsGetParsedWithCorrectPrecedence1);
    FRIEND_TEST(ParserTests, BinaryOperatorExpressionsGetParsedWithCorrectPrecedence2);
    FRIEND_TEST(ParserTests, BracketedBinaryExpressionsGetParsed);
    FRIEND_TEST(ParserTests, GeneratesTheCorrectParsedExpression);
    FRIEND_TEST(ParserTests, ParsesFunctionPrototypeCorrectly);

    Lexer& program;

    std::unique_ptr<ExprAST> parse_number_expr();
    std::unique_ptr<ExprAST> parse_parenthesized_expr();
    std::unique_ptr<ExprAST> parse_identifier_expr();
    std::unique_ptr<ExprAST> parse_primary();
    std::unique_ptr<ExprAST> parse_binary_op_rhs(int, std::unique_ptr<ExprAST>);
    std::unique_ptr<ExprAST> parse_expr();

    std::unique_ptr<FunctionPrototypeAST> parse_function_proto();
    std::unique_ptr<FunctionAST> parse_function_definition();
    std::unique_ptr<FunctionAST> parse_top_level_expr();
    std::unique_ptr<FunctionPrototypeAST> parse_extern();

public:
    Parser(Lexer& lexer);
    virtual ParsedAstContentType get();
    std::vector<ParsedAstContentType> fetch_all();

    Parser operator>>(ParsedAstContentType& ast);
};

}
