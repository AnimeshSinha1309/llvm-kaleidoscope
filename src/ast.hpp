#include <iostream>
#include <memory>
#include <vector>

#include "lexer.hpp"

const std::vector<char> OPERATOR_PRECEDENCE_ORDER = {'*', '-', '+', '<'};

class ExprAST {
public:
    virtual ~ExprAST() = default;
};

class NumberExprAST : public ExprAST {
    double value;

public:
    NumberExprAST(
        double _value
    );
};

class VariableExprAST : public ExprAST {
    std::string name;

public:
    VariableExprAST(
        const std::string &_name
    );
};

class BinaryExprAST : public ExprAST {
    char opcode;
    std::unique_ptr<ExprAST> lhs, rhs;

public:
    BinaryExprAST(
        char _opcode,
        std::unique_ptr<ExprAST> _lhs,
        std::unique_ptr<ExprAST> _rhs
    );
};

class FunctionCallExprAST : public ExprAST {
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;

public:
    FunctionCallExprAST(
        const std::string &_callee,
        std::vector<std::unique_ptr<ExprAST>> _args
    );
};

class FunctionPrototypeAST {
    std::string name;
    std::vector<std::string> args;

public:
    FunctionPrototypeAST(
        const std::string &_name,
        std::vector<std::string> _args
    );
};

class FunctionAST {
    std::unique_ptr<FunctionPrototypeAST> prototype;
    std::unique_ptr<ExprAST> body;

public:
    FunctionAST(
        std::unique_ptr<FunctionPrototypeAST> _prototype,
        std::unique_ptr<ExprAST> _body
    );
};
