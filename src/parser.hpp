#include <deque>
#include <memory>

#include "ast.hpp"
#include "lexer.hpp"

namespace kccani
{

std::unique_ptr<ExprAST> parse_number_expr(
    std::deque<Token>& program
);
std::unique_ptr<ExprAST> parse_parenthesized_expr(
    std::deque<Token>& program
);
std::unique_ptr<ExprAST> parse_identifier_expr(
    std::deque<Token>& program
);
std::unique_ptr<ExprAST> parse_primary(
    std::deque<Token>& program
);

std::unique_ptr<ExprAST> parse_binary_op_rhs(
    std::deque<Token>& program,
    int expression_precedence,
    std::unique_ptr<ExprAST> lhs
);
std::unique_ptr<ExprAST> parse_expr(
    std::deque<Token>& program
);

std::unique_ptr<FunctionPrototypeAST> parse_function_proto(
    std::deque<Token>& program
);
std::unique_ptr<FunctionAST> parse_function_definition(
    std::deque<Token>& program
);
std::unique_ptr<FunctionAST> parse_top_level_expr(
    std::deque<Token>& program
);
std::unique_ptr<FunctionPrototypeAST> parse_extern(
    std::deque<Token>& program
);

std::vector<ParsedAstContentType> parse_program(
    std::deque<Token>& program
);

}
