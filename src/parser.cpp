#include "parser.hpp"

#include <spdlog/spdlog.h>
#include <map>
#include <limits>

namespace kccani
{

// Primary Expression Parsing

std::unique_ptr<ExprAST> parse_number_expr(
    std::deque<Token>& program
)
{
    Token number_token = program.front();
    double number_value = std::get<double>(number_token.data.value());
    auto result = std::make_unique<NumberExprAST>(number_value);
    program.pop_front();
    return std::move(result);
}

std::unique_ptr<ExprAST> parse_parenthesized_expr(
    std::deque<Token>& program
)
{
    program.pop_front();
    auto expression = parse_expr(program);
    if (!expression)
    {
        return nullptr;
    }
    if (!(program.front() == ')'))
    {
        spdlog::error("Expected `)` to close parenthesized expression");
        return nullptr;
    }
    program.pop_front();
    return expression;
}

std::unique_ptr<ExprAST> parse_identifier_expr(
    std::deque<Token>& program
)
{
    // Extract the identifier name
    Token identifier = program.front();
    if (identifier.type != Token::TokenType::TOKEN_IDENTIFIER)
        spdlog::error("Expected token should be an identifier, but is not");
    std::string identifier_name = std::get<std::string>(identifier.data.value());
    program.pop_front();
    // If it's not a function call, process it as a variable name
    if (!(program.front() == '('))
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
        {
            program.pop_front();
            break;
        }
        if (!(program.front() == ','))
        {
            spdlog::error("Expected ')' or ',' after end of expression in argument list");
            return nullptr;
        }
        program.pop_front();
    }
    return std::make_unique<FunctionCallExprAST>(identifier_name, std::move(args));
}

std::unique_ptr<ExprAST> parse_primary(
    std::deque<Token>& program
)
{
    if (program.front().type == Token::TokenType::TOKEN_IDENTIFIER)
    {
        return parse_identifier_expr(program);
    }
    else if (program.front().type == Token::TokenType::TOKEN_NUMBER)
    {
        return parse_number_expr(program);
    }
    else if (program.front() == '(')
    {
        return parse_parenthesized_expr(program);
    }
    else
    {
        spdlog::error("Attempted primary expression parsing non a non-primary expression");
        return nullptr;
    }
}

// Binary expressions and assignments

std::unique_ptr<ExprAST> parse_binary_op_rhs(
    std::deque<Token>& program,
    int expression_precedence,
    std::unique_ptr<ExprAST> lhs
)
{
    const std::map<char, int> OP_PRECEDENCE = {{'<', 100}, {'+', 200}, {'-', 300}, {'*', 400}};

    while (!program.empty() && program.front().type == Token::TokenType::TOKEN_SPECIAL) {
        char opcode = std::get<char>(program.front().data.value());
        int token_precedence = OP_PRECEDENCE.count(opcode) != 0
            ? OP_PRECEDENCE.at(opcode)
            : std::numeric_limits<int>::min();
        if (token_precedence < expression_precedence)
            return std::move(lhs);

        program.pop_front();
        auto rhs = parse_primary(program);
        if (!rhs)
            return nullptr;

        // if opcode binds less tightly with RHS than the operator after RHS,
        // let the pending operator take RHS as its LHS.
        if (!program.empty() && program.front().type == Token::TokenType::TOKEN_SPECIAL)
        {
            char next_opcode = std::get<char>(program.front().data.value());
            int next_precedence = OP_PRECEDENCE.count(next_opcode) != 0
                ? OP_PRECEDENCE.at(next_opcode)
                : std::numeric_limits<int>::min();
            if (token_precedence < next_precedence) {
                rhs = parse_binary_op_rhs(program, token_precedence + 1, std::move(rhs));
                if (!rhs)
                    return nullptr;
            }
        }

        lhs = std::make_unique<BinaryExprAST>(opcode, std::move(lhs), std::move(rhs));
    }
    return std::move(lhs);
}

std::unique_ptr<ExprAST> parse_expr(
    std::deque<Token>& program
) {
    auto lhs = parse_primary(program);
    if (!lhs)
        return nullptr;

    return parse_binary_op_rhs(program, 0, std::move(lhs));
}

// Parsing function blocks and top level (main code in script)

std::unique_ptr<FunctionPrototypeAST> parse_function_proto(
    std::deque<Token>& program
)
{
    if (program.front().type != Token::TokenType::TOKEN_IDENTIFIER)
    {
        spdlog::error("Expected function name in prototype");
        return nullptr;
    }
    std::string function_name = std::get<std::string>(program.front().data.value());
    program.pop_front();

    if (!(program.front() == '('))
    {
        spdlog::error("Expected '(' in prototype");
        return nullptr;
    }
    program.pop_front();

    std::vector<std::string> arguments;
    while (program.front().type == Token::TokenType::TOKEN_IDENTIFIER)
    {
       arguments.push_back(std::get<std::string>(program.front().data.value()));
       program.pop_front();
    }
    if (!(program.front() == ')'))
    {
        spdlog::error("Expected ')' in prototype");
        return nullptr;
    }
    program.pop_front();

    return std::make_unique<FunctionPrototypeAST>(function_name, std::move(arguments));
}

std::unique_ptr<FunctionAST> parse_function_definition(
    std::deque<Token>& program
)
{
    program.pop_front();
    auto prototype = parse_function_proto(program);
    if (!prototype)
        return nullptr;
    auto body = parse_expr(program);
    if (!body)
        return nullptr;
    return std::make_unique<FunctionAST>(std::move(prototype), std::move(body));
}

std::unique_ptr<FunctionAST> parse_top_level_expr(
    std::deque<Token>& program
)
{
    auto expr = parse_expr(program);
    if (!expr)
        return nullptr;

    auto prototype = std::make_unique<FunctionPrototypeAST>(
        "__anon_expr",
        std::vector<std::string>()
    );
    return std::make_unique<FunctionAST>(std::move(prototype), std::move(expr));
}

std::unique_ptr<FunctionPrototypeAST> parse_extern(
    std::deque<Token>& program
)
{
    program.pop_front();
    return parse_function_proto(program);
}

std::vector<std::pair<ParsedAstType, ParsedAstContentType>>
parse_program(std::deque<Token>& program) {
    std::vector<std::pair<ParsedAstType, ParsedAstContentType>> parsed_asts;
    while (true)
    {
        if (program.front().type == Token::TokenType::TOKEN_EOF)
        {
            break;
        }
        else if (program.front() == ';')
        {
            program.pop_front();
        }
        else if (program.front().type == Token::TokenType::TOKEN_DEF)
        {
            ParsedAstContentType defn{parse_function_definition(program)};
            if (std::get<std::unique_ptr<FunctionAST>>(defn) != nullptr)
                parsed_asts.emplace_back(ParsedAstType::DEFINITION, std::move(defn));
        }
        else if (program.front().type == Token::TokenType::TOKEN_EXTERN)
        {
            ParsedAstContentType call{parse_extern(program)};
            if (std::get<std::unique_ptr<FunctionPrototypeAST>>(call) != nullptr)
                parsed_asts.emplace_back(ParsedAstType::EXTERN, std::move(call));
        }
        else
        {
            ParsedAstContentType expr{parse_expr(program)};
            if (std::get<std::unique_ptr<ExprAST>>(expr) != nullptr)
                parsed_asts.emplace_back(ParsedAstType::EXPRESSION, std::move(expr));
        }
    }
    return parsed_asts; 
}

}
