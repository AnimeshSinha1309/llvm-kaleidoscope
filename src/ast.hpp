#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "lexer.hpp"

namespace kccani
{

const std::vector<char> OPERATOR_PRECEDENCE_ORDER = {'*', '-', '+', '<'};

class ExprAST {
public:
    virtual ~ExprAST() = default;
};

class NumberExprAST : public ExprAST {
public:
    double value;

    NumberExprAST(
        double _value
    );
};

class VariableExprAST : public ExprAST {
public:
    std::string name;

    VariableExprAST(
        const std::string &_name
    );
};

class BinaryExprAST : public ExprAST {
public:
    char opcode;
    std::unique_ptr<ExprAST> lhs, rhs;

    BinaryExprAST(
        char _opcode,
        std::unique_ptr<ExprAST> _lhs,
        std::unique_ptr<ExprAST> _rhs
    );
};

class FunctionCallExprAST : public ExprAST {
public:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;

    FunctionCallExprAST(
        const std::string &_callee,
        std::vector<std::unique_ptr<ExprAST>> _args
    );
};

class FunctionPrototypeAST {
public:
    std::string name;
    std::vector<std::string> args;

    FunctionPrototypeAST(
        const std::string &_name,
        std::vector<std::string> _args
    );
};

class FunctionAST {
public:
    std::unique_ptr<FunctionPrototypeAST> prototype;
    std::unique_ptr<ExprAST> body;

    FunctionAST(
        std::unique_ptr<FunctionPrototypeAST> _prototype,
        std::unique_ptr<ExprAST> _body
    );
};

}
