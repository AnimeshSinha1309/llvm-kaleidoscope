#include <deque>
#include <memory>

#include "ast.hpp"
#include "lexer.hpp"

namespace kccani
{

std::unique_ptr<ExprAST> parse_number_expr(
    Lexer& program
);
std::unique_ptr<ExprAST> parse_parenthesized_expr(
    Lexer& program
);
std::unique_ptr<ExprAST> parse_identifier_expr(
    Lexer& program
);
std::unique_ptr<ExprAST> parse_primary(
    Lexer& program
);

std::unique_ptr<ExprAST> parse_binary_op_rhs(
    Lexer& program,
    int expression_precedence,
    std::unique_ptr<ExprAST> lhs
);
std::unique_ptr<ExprAST> parse_expr(
    Lexer& program
);

std::unique_ptr<FunctionPrototypeAST> parse_function_proto(
    Lexer& program
);
std::unique_ptr<FunctionAST> parse_function_definition(
    Lexer& program
);
std::unique_ptr<FunctionAST> parse_top_level_expr(
    Lexer& program
);
std::unique_ptr<FunctionPrototypeAST> parse_extern(
    Lexer& program
);

std::vector<ParsedAstContentType> parse_program(
    Lexer& program
);

}
