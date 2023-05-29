#include "ast.hpp"
#include "error.hpp"

#include <cassert>
#include <variant>

NumberExprAST::NumberExprAST(
    double _value
) : value(_value) {}

VariableExprAST::VariableExprAST(
    const std::string &_name
) : name(_name) {}

BinaryExprAST::BinaryExprAST(
    char _opcode,
    std::unique_ptr<ExprAST> _lhs,
    std::unique_ptr<ExprAST> _rhs
) : opcode(_opcode), lhs(std::move(_lhs)), rhs(std::move(_rhs)) {}

FunctionCallExprAST::FunctionCallExprAST(
    const std::string &_callee,
    std::vector<std::unique_ptr<ExprAST>> args
) : callee(_callee), args(std::move(args)) {}

FunctionPrototypeAST::FunctionPrototypeAST(
    const std::string &_name,
    std::vector<std::string> _args
) : name(_name), args(std::move(_args)) {}

FunctionAST::FunctionAST(
    std::unique_ptr<FunctionPrototypeAST> _prototype,
    std::unique_ptr<ExprAST> _body
) : prototype(std::move(_prototype)), body(std::move(_body)) {}

