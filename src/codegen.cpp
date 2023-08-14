#include "codegen.hpp"

#include <spdlog/spdlog.h>

namespace kccani
{

llvm::Value* CodeGeneratorLLVM::codegen_expr(std::unique_ptr<ExprAST>&& ast)
{
    switch (ast->get_type())
    {
    case ExprAST::ExpressionType::NUMBER_EXPR:
    {
        auto expr = std::unique_ptr<NumberExprAST>(static_cast<NumberExprAST*>(ast.release()));
        return llvm::ConstantFP::get(*this->context, llvm::APFloat(expr->value));
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
        llvm::Value* l = this->codegen_expr(std::move(expr->lhs));
        llvm::Value* r = this->codegen_expr(std::move(expr->rhs));
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

        llvm::Function *callee_func = this->get_function(expr->callee);
        if (!callee_func)
        {
            spdlog::error("Undefined function with name: " + expr->callee);
            return nullptr;
        }

        if (callee_func->arg_size() != expr->args.size())
        {
            spdlog::error("Incorrect number of arguments passed");
            return nullptr;
        }
        std::vector<llvm::Value*> args_llvm_values;
        for (unsigned i = 0, e = expr->args.size(); i != e; ++i) {
            args_llvm_values.push_back(this->codegen_expr(std::move(expr->args[i])));
            if (!args_llvm_values.back())
                return nullptr;
        }

        return this->builder->CreateCall(callee_func, args_llvm_values, "calltmp");
    }
    default:
        spdlog::error("Invalid expression type, all expression types should be in switch above");
        return nullptr;
    }
}

llvm::Function* CodeGeneratorLLVM::get_function(const std::string& name)
{
    return this->module->getFunction(name);
}


llvm::Function* CodeGeneratorLLVM::operator()(std::unique_ptr<ExprAST>&& ast)
{
    auto fn_proto = std::make_unique<FunctionPrototypeAST>("__anon_expr", std::vector<std::string>());
    auto fn_ast = std::make_unique<FunctionAST>(std::move(fn_proto), std::move(ast));
    auto fn_llvm_ir = (*this)(std::move(fn_ast));
    fn_llvm_ir->eraseFromParent();
    return fn_llvm_ir;
}

llvm::Function* CodeGeneratorLLVM::operator()(std::unique_ptr<FunctionAST>&& ast)
{
    // First, check for an existing function from a previous 'extern' declaration.
    llvm::Function* the_function = this->get_function(ast->prototype->name);
    if (!the_function)
        the_function = (*this)(std::move(ast->prototype));
    if (!the_function)
        return nullptr;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *basic_block = llvm::BasicBlock::Create(*this->context, "entry", the_function);
    this->builder->SetInsertPoint(basic_block);

    // Record the function arguments in the NamedValues map.
    this->named_values.clear();
    for (auto &arg : the_function->args())
    this->named_values[std::string(arg.getName())] = &arg;

    if (llvm::Value *return_value = this->codegen_expr(std::move(ast->body))) {
        // Finish off the function.
        this->builder->CreateRet(return_value);
        // Validate the generated code, checking for consistency.
        llvm::verifyFunction(*the_function);
        return the_function;
    }
    // Error reading body, remove function.
    the_function->eraseFromParent();
    return nullptr;
}

llvm::Function* CodeGeneratorLLVM::operator()(std::unique_ptr<FunctionPrototypeAST>&& ast)
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

llvm::Function* CodeGeneratorLLVM::operator()(std::monostate&& invalid)
{
    return nullptr;
}

void CodeGeneratorLLVM::print() const
{
    this->module->print(llvm::errs(), nullptr);
}

std::string CodeGeneratorLLVM::to_string() const
{
    std::string llvm_output;
    llvm::raw_string_ostream rso(llvm_output);
    this->module->print(rso, nullptr);
    return llvm_output;
}

void CodeGeneratorLLVM::print(llvm::Function* generated_code)
{
    generated_code->print(llvm::errs());
}

std::string CodeGeneratorLLVM::to_string(llvm::Function* generated_code)
{
    std::string llvm_output;
    llvm::raw_string_ostream rso(llvm_output);
    generated_code->print(rso);
    return llvm_output;
}

}
