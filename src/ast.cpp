#include "ast.hpp"
#include "error.hpp"

#include <cassert>
#include <variant>

namespace kccani
{

NumberExprAST::NumberExprAST(
    double _value
) : value(_value) {}

std::string NumberExprAST::to_string()
{
    return std::to_string(this->value);
}

VariableExprAST::VariableExprAST(
    const std::string &_name
) : name(_name) {}

std::string VariableExprAST::to_string() 
{
    return this->name;
}

BinaryExprAST::BinaryExprAST(
    char _opcode,
    std::unique_ptr<ExprAST> _lhs,
    std::unique_ptr<ExprAST> _rhs
) : opcode(_opcode), lhs(std::move(_lhs)), rhs(std::move(_rhs)) {}

std::string BinaryExprAST::to_string()
{
    return "(" + this->lhs->to_string() + ") " + this->opcode + 
        " (" + this->rhs->to_string() + ")";
}

FunctionCallExprAST::FunctionCallExprAST(
    const std::string &_callee,
    std::vector<std::unique_ptr<ExprAST>> args
) : callee(_callee), args(std::move(args)) {}

std::string FunctionCallExprAST::to_string()
{
    std::string args_string = "";
    for (int i = 0; i < this->args.size(); i++)
    {
        args_string += this->args[i]->to_string();
        if (i != this->args.size() - 1) {
            args_string += ", ";
        }
    }
    return this->callee + "(" + args_string + ")";
}

FunctionPrototypeAST::FunctionPrototypeAST(
    const std::string &_name,
    std::vector<std::string> _args
) : name(_name), args(std::move(_args)) {}

std::string FunctionPrototypeAST::to_string()
{
    std::string args_string = "";
    for (int i = 0; i < this->args.size(); i++)
    {
        args_string += this->args[i];
        if (i != this->args.size() - 1) {
            args_string += ", ";
        }
    }
    return "def " + this->name + "(" + args_string + ")";
}

FunctionAST::FunctionAST(
    std::unique_ptr<FunctionPrototypeAST> _prototype,
    std::unique_ptr<ExprAST> _body
) : prototype(std::move(_prototype)), body(std::move(_body)) {}

std::string FunctionAST::to_string()
{
    return this->prototype->to_string() + "{" + this->body->to_string() + "}";
}

}
