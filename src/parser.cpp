#include "parser.hpp"

#include <spdlog/spdlog.h>

namespace kccani
{

static std::unique_ptr<ExprAST> parse_expr(
    std::deque<Token> program
);
static std::unique_ptr<ExprAST> parse_number_expr(
    std::deque<Token> program
);
static std::unique_ptr<ExprAST> parse_parenthesized_expr(
    std::deque<Token> program
);
static std::unique_ptr<ExprAST> parse_identifier_expr(
    std::deque<Token> program
);


static std::unique_ptr<ExprAST> parse_expr(
    std::deque<Token> program
)
{
    return nullptr;
}

static std::unique_ptr<ExprAST> parse_number_expr(
    std::deque<Token> program
)
{
    Token number_token = program.front();
    auto result = std::make_unique<NumberExprAST>(number_token);
    program.pop_front();
    return std::move(result);
}

static std::unique_ptr<ExprAST> parse_parenthesized_expr(
    std::deque<Token> program
)
{
    program.pop_front();
    auto expression = parse_expr(program);
    if (!expression)
    {
        return nullptr;
    }
    if (program.front() != ')')
    {
        spdlog::error("Expected `)` to close parenthesized expression");
        return nullptr;
    }
    program.pop_front();
    return expression;
}

static std::unique_ptr<ExprAST> parse_identifier_expr(
    std::deque<Token> program
)
{
    // Extract the identifier name
    Token identifier = program.front();
    if (identifier.type != Token::TokenType::TOKEN_IDENTIFIER)
        spdlog::error("Expected token should be an identifier, but is not");
    std::string identifier_name = std::get<std::string>(identifier.data.value());
    program.pop_front();
    // If it's not a function call, process it as a variable name
    if (program.front() != '(')
    {
        return std::make_unique<VariableExprAST>(identifier_name);
    }
    // If it is a function call and has no args, process it as such
    program.pop_front();
    std::vector<std::unique_ptr<ExprAST>> function_args;
    if (program.front() == ')')
    {
        program.pop_front();
        return std::make_unique<FunctionCallExprAST>(
            identifier_name, std::move(function_args));
    }
    // Otherwise collect args until close parenthesis, process that
    std::vector<std::unique_ptr<ExprAST>> args;
    while (true)
    {
        if (auto arg = parse_expr(program))
            args.push_back(std::move(arg));
        else
            return nullptr;

        if (program.front() == ')')
            break;
        if (program.front() != ',')
        {
            spdlog::error("Expected ')' or ',' after end of expression in argument list");
            return nullptr;
        }
        program.pop_front();
    }
    return std::make_unique<FunctionCallExprAST>(identifier_name, std::move(args));
}

}
