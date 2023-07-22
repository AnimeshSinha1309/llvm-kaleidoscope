#include "parser.hpp"

#include <spdlog/spdlog.h>
#include <map>
#include <limits>

namespace kccani
{

// Primary Expression Parsing

std::unique_ptr<ExprAST> parse_number_expr(
    Lexer& program
)
{
    Token number_token = program.get();
    double number_value = std::get<double>(number_token.data.value());
    auto result = std::make_unique<NumberExprAST>(number_value);
    return std::move(result);
}

std::unique_ptr<ExprAST> parse_parenthesized_expr(
    Lexer& program
)
{
    program.get();
    auto expression = parse_expr(program);
    if (!expression)
    {
        return nullptr;
    }
    if (!(program.peek() == ')'))
    {
        spdlog::error("Expected `)` to close parenthesized expression");
        return nullptr;
    }
    program.get();
    return expression;
}

std::unique_ptr<ExprAST> parse_identifier_expr(
    Lexer& program
)
{
    // Extract the identifier name
    Token identifier = program.get();
    if (identifier.type != Token::TokenType::TOKEN_IDENTIFIER)
        spdlog::error("Expected token should be an identifier, but is not");
    std::string identifier_name = std::get<std::string>(identifier.data.value());
    // If it's not a function call, process it as a variable name
    if (!(program.peek() == '('))
    {
        return std::make_unique<VariableExprAST>(identifier_name);
    }
    // If it is a function call and has no args, process it as such
    program.get();
    std::vector<std::unique_ptr<ExprAST>> function_args;
    if (program.peek() == ')')
    {
        program.get();
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

        if (program.peek() == ')')
        {
            program.get();
            break;
        }
        if (!(program.peek() == ','))
        {
            spdlog::error("Expected ')' or ',' after end of expression in argument list");
            return nullptr;
        }
        program.get();
    }
    return std::make_unique<FunctionCallExprAST>(identifier_name, std::move(args));
}

std::unique_ptr<ExprAST> parse_primary(
    Lexer& program
)
{
    if (program.peek().type == Token::TokenType::TOKEN_IDENTIFIER)
    {
        return parse_identifier_expr(program);
    }
    else if (program.peek().type == Token::TokenType::TOKEN_NUMBER)
    {
        return parse_number_expr(program);
    }
    else if (program.peek() == '(')
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
    Lexer& program,
    int expression_precedence,
    std::unique_ptr<ExprAST> lhs
)
{
    const std::map<char, int> OP_PRECEDENCE = {{'<', 100}, {'+', 200}, {'-', 300}, {'*', 400}};

    while (program.peek().type == Token::TokenType::TOKEN_SPECIAL) {
        char opcode = std::get<char>(program.peek().data.value());
        int token_precedence = OP_PRECEDENCE.count(opcode) != 0
            ? OP_PRECEDENCE.at(opcode)
            : std::numeric_limits<int>::min();
        if (token_precedence < expression_precedence)
            return std::move(lhs);
        program.get();

        auto rhs = parse_primary(program);
        if (!rhs)
            return nullptr;

        // if opcode binds less tightly with RHS than the operator after RHS,
        // let the pending operator take RHS as its LHS.
        if (program.peek().type == Token::TokenType::TOKEN_SPECIAL)
        {
            char next_opcode = std::get<char>(program.peek().data.value());
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
    Lexer& program
) {
    auto lhs = parse_primary(program);
    if (!lhs)
        return nullptr;

    return parse_binary_op_rhs(program, 0, std::move(lhs));
}

// Parsing function blocks and top level (main code in script)

std::unique_ptr<FunctionPrototypeAST> parse_function_proto(
    Lexer& program
)
{
    if (program.peek().type != Token::TokenType::TOKEN_IDENTIFIER)
    {
        spdlog::error("Expected function name in prototype");
        return nullptr;
    }
    std::string function_name = std::get<std::string>(program.get().data.value());

    if (!(program.get() == '('))
    {
        spdlog::error("Expected '(' in prototype");
        return nullptr;
    }

    std::vector<std::string> arguments;
    while (program.peek().type == Token::TokenType::TOKEN_IDENTIFIER)
    {
       arguments.push_back(std::get<std::string>(program.get().data.value()));
    }
    if (!(program.get() == ')'))
    {
        spdlog::error("Expected ')' in prototype");
        return nullptr;
    }

    return std::make_unique<FunctionPrototypeAST>(function_name, std::move(arguments));
}

std::unique_ptr<FunctionAST> parse_function_definition(
    Lexer& program
)
{
    program.get();
    auto prototype = parse_function_proto(program);
    if (!prototype)
        return nullptr;
    auto body = parse_expr(program);
    if (!body)
        return nullptr;
    return std::make_unique<FunctionAST>(std::move(prototype), std::move(body));
}

std::unique_ptr<FunctionAST> parse_top_level_expr(
    Lexer& program
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
    Lexer& program
)
{
    program.get();
    return parse_function_proto(program);
}

std::vector<ParsedAstContentType> parse_program(Lexer& program) {
    std::vector<ParsedAstContentType> parsed_asts;
    while (true)
    {
        if (program.peek().type == Token::TokenType::TOKEN_EOF)
        {
            break;
        }
        else if (program.peek() == ';')
        {
            program.get();
        }
        else if (program.peek().type == Token::TokenType::TOKEN_DEF)
        {
            ParsedAstContentType defn{parse_function_definition(program)};
            if (std::get<std::unique_ptr<FunctionAST>>(defn) != nullptr)
                parsed_asts.emplace_back(std::move(defn));
        }
        else if (program.peek().type == Token::TokenType::TOKEN_EXTERN)
        {
            ParsedAstContentType call{parse_extern(program)};
            if (std::get<std::unique_ptr<FunctionPrototypeAST>>(call) != nullptr)
                parsed_asts.emplace_back(std::move(call));
        }
        else
        {
            ParsedAstContentType expr{parse_expr(program)};
            if (std::get<std::unique_ptr<ExprAST>>(expr) != nullptr)
                parsed_asts.emplace_back(std::move(expr));
        }
    }
    return parsed_asts; 
}

}
