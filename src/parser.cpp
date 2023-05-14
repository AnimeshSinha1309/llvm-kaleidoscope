#include "parser.hpp"

NumberExpressionAST::NumberExpressionAST(
    double _value
) : value(_value) {}

VariableExpressionAST::VariableExpressionAST(
    const std::string &_name
) : name(_name) {}

BinaryExprAST::BinaryExprAST(
    char _opcode,
    std::unique_ptr<ExpressionAST> _lhs,
    std::unique_ptr<ExpressionAST> _rhs
) : opcode(_opcode), lhs(std::move(_lhs)), rhs(std::move(_rhs)) {}

FunctionCallExpressionAST::FunctionCallExpressionAST(
    const std::string &_callee,
    std::vector<std::unique_ptr<ExpressionAST>> args
) : callee(_callee), args(std::move(args)) {}
