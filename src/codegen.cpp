#include "codegen.hpp"

#include <spdlog/spdlog.h>

namespace kccani

{

CodegenContentType CodeGeneratorLLVM::operator()(std::unique_ptr<ExprAST>&& ast)
{
    switch (ast->get_type())
    {
    case ExprAST::ExpressionType::NUMBER_EXPR:
    {
        auto expr = std::unique_ptr<NumberExprAST>(static_cast<NumberExprAST*>(ast.release()));
        return llvm::ConstantFP::get(*this->context, llvm::APFloat(3.0));
    }
    case ExprAST::ExpressionType::VARIABLE_EXPR:
    {
        auto expr = std::unique_ptr<VariableExprAST>(static_cast<VariableExprAST*>(ast.release()));
        llvm::Value *value = this->named_values[expr->name];
        if (!value)
            spdlog::error("Unknown variable name: " + expr->name);
        return value;
    }
    case ExprAST::ExpressionType::BINARY_EXPR:
    {
        auto expr = std::unique_ptr<BinaryExprAST>(static_cast<BinaryExprAST*>(ast.release()));
        llvm::Value* l = std::get<llvm::Value*>((*this)(std::move(expr->lhs)));
        llvm::Value* r = std::get<llvm::Value*>((*this)(std::move(expr->rhs)));
        if (!l || !r)
            return (llvm::Value*) nullptr;
        switch (expr->opcode)
        {
        case '+':
            return this->builder->CreateFAdd(l, r, "addtmp");
        case '-':
            return this->builder->CreateFSub(l, r, "subtmp");
        case '*':
            return this->builder->CreateFMul(l, r, "multmp");
        case '<':
            return this->builder->CreateUIToFP(
                this->builder->CreateFCmpULT(l, r, "cmptmp"),
                llvm::Type::getDoubleTy(*(this->context)),
                "booltmp"
            );
        default:
            spdlog::error("The opcode for the binary operation was not recognized");
            return (llvm::Value*) nullptr;
        }
    }
    case ExprAST::ExpressionType::FUNCTION_CALL_EXPR:
    {
        auto expr = std::unique_ptr<FunctionCallExprAST>(static_cast<FunctionCallExprAST*>(ast.release()));

        llvm::Function *callee_func = this->module->getFunction(expr->callee);
        if (!callee_func)
        {
            spdlog::error("Undefined function with name: " + expr->callee);
            return (llvm::Value*) nullptr;
        }

        if (callee_func->arg_size() != expr->args.size())
        {
            spdlog::error("Incorrect number of arguments passed");
            return (llvm::Value*) nullptr;
        }
        std::vector<llvm::Value*> args_llvm_values;
        for (unsigned i = 0, e = expr->args.size(); i != e; ++i) {
            args_llvm_values.push_back(std::get<llvm::Value*>((*this)(std::move(expr->args[i]))));
            if (!args_llvm_values.back())
                return (llvm::Value*) nullptr;
        }

        return this->builder->CreateCall(callee_func, args_llvm_values, "calltmp");
    }
    default:
        spdlog::error("Invalid expression type, all expression types should be in switch above");
        return (llvm::Value*) nullptr;
    }
}

CodegenContentType CodeGeneratorLLVM::operator()(std::unique_ptr<FunctionAST>&& ast)
{
    // First, check for an existing function from a previous 'extern' declaration.
    llvm::Function *the_function = this->module->getFunction(ast->prototype->name);
    if (!the_function)
        the_function = std::get<llvm::Function*>((*this)(std::move(ast->prototype)));
    if (!the_function)
        return (llvm::Function*) nullptr;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *basic_block = llvm::BasicBlock::Create(*this->context, "entry", the_function);
    this->builder->SetInsertPoint(basic_block);

    // Record the function arguments in the NamedValues map.
    this->named_values.clear();
    for (auto &arg : the_function->args())
    this->named_values[std::string(arg.getName())] = &arg;

    if (llvm::Value *return_value = std::get<llvm::Value*>((*this)(std::move(ast->body)))) {
        // Finish off the function.
        this->builder->CreateRet(return_value);
        // Validate the generated code, checking for consistency.
        llvm::verifyFunction(*the_function);
        return the_function;
    }
    // Error reading body, remove function.
    the_function->eraseFromParent();
    return (llvm::Function*) nullptr;
}

CodegenContentType CodeGeneratorLLVM::operator()(std::unique_ptr<FunctionPrototypeAST>&& ast)
{
    std::vector<llvm::Type*> doubles(ast->args.size(), llvm::Type::getDoubleTy(*this->context));
    llvm::FunctionType *function_type = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*this->context), doubles, false);
    llvm::Function *function = llvm::Function::Create(
        function_type, llvm::Function::ExternalLinkage, ast->name, this->module.get());

    uint32_t idx = 0;
    for (auto &arg : function->args())
        arg.setName(ast->args[idx++]);

    return function;
}

CodegenContentType CodeGeneratorLLVM::operator()(std::monostate&& invalid)
{
    return std::monostate{};
}

void CodeGeneratorLLVM::print() const
{
    this->module->print(llvm::errs(), nullptr);
}

void CodeGeneratorLLVM::print(CodegenContentType generated_code)
{
    if (std::holds_alternative<llvm::Function*>(generated_code))
        std::get<llvm::Function*>(generated_code)->print(llvm::errs());
    else if (std::holds_alternative<llvm::Value*>(generated_code))
        std::get<llvm::Value*>(generated_code)->print(llvm::errs());
    else if (std::holds_alternative<std::monostate>(generated_code))
        std::cout << "Error in Code Generation" << std::endl;
}

}
