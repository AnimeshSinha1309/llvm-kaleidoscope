#include "parser.hpp"

#include <spdlog/spdlog.h>
#include <map>
#include <limits>

namespace kccani
{

// Primary Expression Parsing

std::unique_ptr<ExprAST> Parser::parse_number_expr()
{
    Token number_token = this->program.get();
    double number_value = std::get<double>(number_token.data.value());
    auto result = std::make_unique<NumberExprAST>(number_value);
    return std::move(result);
}

std::unique_ptr<ExprAST> Parser::parse_parenthesized_expr()
{
    this->program.get();
    auto expression = this->parse_expr();
    if (!expression)
    {
        return nullptr;
    }
    if (!(this->program.peek() == ')'))
    {
        spdlog::error("Expected `)` to close parenthesized expression");
        return nullptr;
    }
    this->program.get();
    return expression;
}

std::unique_ptr<ExprAST> Parser::parse_identifier_expr()
{
    // Extract the identifier name
    Token identifier = this->program.get();
    if (identifier.type != Token::TokenType::TOKEN_IDENTIFIER)
        spdlog::error("Expected token should be an identifier, but is not");
    std::string identifier_name = std::get<std::string>(identifier.data.value());
    // If it's not a function call, process it as a variable name
    if (!(this->program.peek() == '('))
    {
        return std::make_unique<VariableExprAST>(identifier_name);
    }
    // If it is a function call and has no args, process it as such
    this->program.get();
    std::vector<std::unique_ptr<ExprAST>> function_args;
    if (this->program.peek() == ')')
    {
        this->program.get();
        return std::make_unique<FunctionCallExprAST>(
            identifier_name, std::move(function_args));
    }
    // Otherwise collect args until close parenthesis, process that
    std::vector<std::unique_ptr<ExprAST>> args;
    while (true)
    {
        if (auto arg = this->parse_expr())
            args.push_back(std::move(arg));
        else
            return nullptr;

        if (this->program.peek() == ')')
        {
            this->program.get();
            break;
        }
        if (!(this->program.peek() == ','))
        {
            spdlog::error("Expected ')' or ',' after end of expression in argument list");
            return nullptr;
        }
        this->program.get();
    }
    return std::make_unique<FunctionCallExprAST>(identifier_name, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parse_primary()
{
    if (this->program.peek().type == Token::TokenType::TOKEN_IDENTIFIER)
    {
        return this->parse_identifier_expr();
    }
    else if (this->program.peek().type == Token::TokenType::TOKEN_NUMBER)
    {
        return this->parse_number_expr();
    }
    else if (this->program.peek() == '(')
    {
        return this->parse_parenthesized_expr();
    }
    else
    {
        spdlog::error("Attempted primary expression parsing non a non-primary expression");
        return nullptr;
    }
}

// Binary expressions and assignments

std::unique_ptr<ExprAST> Parser::parse_binary_op_rhs(
    int expression_precedence,
    std::unique_ptr<ExprAST> lhs
)
{
    const std::map<char, int> OP_PRECEDENCE = {{'<', 100}, {'+', 200}, {'-', 300}, {'*', 400}};

    while (this->program.peek().type == Token::TokenType::TOKEN_SPECIAL) {
        char opcode = std::get<char>(this->program.peek().data.value());
        int token_precedence = OP_PRECEDENCE.count(opcode) != 0
            ? OP_PRECEDENCE.at(opcode)
            : std::numeric_limits<int>::min();
        if (token_precedence < expression_precedence)
            return std::move(lhs);
        this->program.get();

        auto rhs = this->parse_primary();
        if (!rhs)
            return nullptr;

        // if opcode binds less tightly with RHS than the operator after RHS,
        // let the pending operator take RHS as its LHS.
        if (this->program.peek().type == Token::TokenType::TOKEN_SPECIAL)
        {
            char next_opcode = std::get<char>(this->program.peek().data.value());
            int next_precedence = OP_PRECEDENCE.count(next_opcode) != 0
                ? OP_PRECEDENCE.at(next_opcode)
                : std::numeric_limits<int>::min();
            if (token_precedence < next_precedence) {
                rhs = this->parse_binary_op_rhs(token_precedence + 1, std::move(rhs));
                if (!rhs)
                    return nullptr;
            }
        }

        lhs = std::make_unique<BinaryExprAST>(opcode, std::move(lhs), std::move(rhs));
    }
    return std::move(lhs);
}

std::unique_ptr<ExprAST> Parser::parse_expr() {
    auto lhs = this->parse_primary();
    if (!lhs)
        return nullptr;

    return this->parse_binary_op_rhs(0, std::move(lhs));
}

// Parsing function blocks and top level (main code in script)

std::unique_ptr<FunctionPrototypeAST> Parser::parse_function_proto()
{
    if (this->program.peek().type != Token::TokenType::TOKEN_IDENTIFIER)
    {
        spdlog::error("Expected function name in prototype");
        return nullptr;
    }
    std::string function_name = std::get<std::string>(this->program.get().data.value());

    if (!(this->program.get() == '('))
    {
        spdlog::error("Expected '(' in prototype");
        return nullptr;
    }

    std::vector<std::string> arguments;
    while (this->program.peek().type == Token::TokenType::TOKEN_IDENTIFIER)
    {
       arguments.push_back(std::get<std::string>(this->program.get().data.value()));
    }
    if (!(this->program.get() == ')'))
    {
        spdlog::error("Expected ')' in prototype");
        return nullptr;
    }

    return std::make_unique<FunctionPrototypeAST>(function_name, std::move(arguments));
}

std::unique_ptr<FunctionAST> Parser::parse_function_definition()
{
    this->program.get();
    auto prototype = this->parse_function_proto();
    if (!prototype)
        return nullptr;
    auto body = this->parse_expr();
    if (!body)
        return nullptr;
    return std::make_unique<FunctionAST>(std::move(prototype), std::move(body));
}

std::unique_ptr<FunctionAST> Parser::parse_top_level_expr()
{
    auto expr = this->parse_expr();
    if (!expr)
        return nullptr;

    auto prototype = std::make_unique<FunctionPrototypeAST>(
        "__anon_expr",
        std::vector<std::string>()
    );
    return std::make_unique<FunctionAST>(std::move(prototype), std::move(expr));
}

std::unique_ptr<FunctionPrototypeAST> Parser::parse_extern()
{
    this->program.get();
    return this->parse_function_proto();
}

Parser::Parser(Lexer& lexer) : program(lexer)
{
}

ParsedAstContentType Parser::get() {
    if (this->program.peek().type != Token::TokenType::TOKEN_EOF)
    {
        if (this->program.peek() == ';')
        {
            this->program.get();
            return std::move(this->get());
        }
        else if (this->program.peek().type == Token::TokenType::TOKEN_DEF)
        {
            ParsedAstContentType defn{this->parse_function_definition()};
            if (std::get<std::unique_ptr<FunctionAST>>(defn) != nullptr)
                return std::move(defn);
        }
        else if (this->program.peek().type == Token::TokenType::TOKEN_EXTERN)
        {
            ParsedAstContentType call{this->parse_extern()};
            if (std::get<std::unique_ptr<FunctionPrototypeAST>>(call) != nullptr)
                return std::move(call);
        }
        else
        {
            ParsedAstContentType expr{this->parse_expr()};
            if (std::get<std::unique_ptr<ExprAST>>(expr) != nullptr)
                return std::move(expr);
        }
    }
    // EOF or error token
    return ParsedAstContentType{std::unique_ptr<FunctionAST>{nullptr}};
}

std::vector<ParsedAstContentType> Parser::fetch_all()
{
    std::vector<ParsedAstContentType> ast_list;
    while (this->program.peek().type != Token::TokenType::TOKEN_EOF)
    {
        ast_list.push_back(this->get());
    }
    return ast_list;
}

Parser Parser::operator>>(ParsedAstContentType& ast)
{
    ast = this->get();
    return *this;
}

}
