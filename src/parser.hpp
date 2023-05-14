#include <iostream>
#include <memory>
#include <vector>

class ExpressionAST {
public:
    virtual ~ExpressionAST() = default;
};

class NumberExpressionAST : public ExpressionAST {
    double value;

public:
    NumberExpressionAST(double _value);
};

class VariableExpressionAST : public ExpressionAST {
    std::string name;

public:
    VariableExpressionAST(const std::string &_name);
};

class BinaryExprAST : public ExpressionAST {
    char opcode;
    std::unique_ptr<ExpressionAST> lhs, rhs;

public:
    BinaryExprAST(
        char _opcode,
        std::unique_ptr<ExpressionAST> _lhs,
        std::unique_ptr<ExpressionAST> _rhs
    );
};

class FunctionCallExpressionAST : public ExpressionAST {
    std::string callee;
    std::vector<std::unique_ptr<ExpressionAST>> args;

public:
    FunctionCallExpressionAST(
        const std::string &_callee,
        std::vector<std::unique_ptr<ExpressionAST>> args
    );
};
