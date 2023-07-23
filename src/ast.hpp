#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "lexer.hpp"

namespace kccani
{

const std::vector<char> OPERATOR_PRECEDENCE_ORDER = {'*', '-', '+', '<'};

class ExprAST
{
public:
    enum class ExpressionType
    {
        NUMBER_EXPR,
        VARIABLE_EXPR,
        BINARY_EXPR,
        FUNCTION_CALL_EXPR,
    };

    virtual ~ExprAST() = default;

    virtual std::string to_string() = 0;
    virtual ExpressionType get_type() = 0;
};

class NumberExprAST : public ExprAST
{
public:
    double value;

    NumberExprAST(
        double _value
    );

    std::string to_string();
    ExprAST::ExpressionType get_type() override;
};

class VariableExprAST : public ExprAST
{
public:
    std::string name;

    VariableExprAST(
        const std::string &_name
    );

    std::string to_string();
    ExprAST::ExpressionType get_type() override;
};

class BinaryExprAST : public ExprAST
{
public:
    char opcode;
    std::unique_ptr<ExprAST> lhs, rhs;

    BinaryExprAST(
        char _opcode,
        std::unique_ptr<ExprAST> _lhs,
        std::unique_ptr<ExprAST> _rhs
    );

    std::string to_string();
    ExprAST::ExpressionType get_type() override;
};

class FunctionCallExprAST : public ExprAST
{
public:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;

    FunctionCallExprAST(
        const std::string &_callee,
        std::vector<std::unique_ptr<ExprAST>> _args
    );

    std::string to_string();
    ExprAST::ExpressionType get_type() override;
};

class FunctionPrototypeAST
{
public:
    std::string name;
    std::vector<std::string> args;

    FunctionPrototypeAST(
        const std::string &_name,
        std::vector<std::string> _args
    );

    std::string to_string();
};

class FunctionAST
{
public:
    std::unique_ptr<FunctionPrototypeAST> prototype;
    std::unique_ptr<ExprAST> body;

    FunctionAST(
        std::unique_ptr<FunctionPrototypeAST> _prototype,
        std::unique_ptr<ExprAST> _body
    );

    std::string to_string();
};

using ParsedAstContentType = std::variant<
    std::unique_ptr<FunctionAST>,
    std::unique_ptr<FunctionPrototypeAST>,
    std::unique_ptr<ExprAST>,
    std::monostate>;

}
