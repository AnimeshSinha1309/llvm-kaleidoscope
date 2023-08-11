#pragma once

#include <map>
#include <memory>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include "ast.hpp"

namespace kccani
{

class CodeGeneratorLLVM
{
protected:
    std::unique_ptr<llvm::LLVMContext> context{std::make_unique<llvm::LLVMContext>()};
    std::unique_ptr<llvm::IRBuilder<>> builder{std::make_unique<llvm::IRBuilder<>>(*context)};
    std::unique_ptr<llvm::Module> module{std::make_unique<llvm::Module>("kccani_jit", *context)};
    std::map<std::string, llvm::Value*> named_values;

    // Evaluating the bodies of functions down to a value
    llvm::Value* codegen_expr(std::unique_ptr<ExprAST>&& ast);

public:
    // Handing top-level expressions
    llvm::Function* operator()(std::unique_ptr<ExprAST>&& ast);
    // Handling function definitions
    llvm::Function* operator()(std::unique_ptr<FunctionAST>&& ast);
    // Handling external linkage
    llvm::Function* operator()(std::unique_ptr<FunctionPrototypeAST>&& ast);
    // Handling statements with parsing errors in them
    llvm::Function* operator()(std::monostate&& ast);

    void print() const;
    std::string to_string() const;
    static void print(llvm::Function* generated_code);
    static std::string to_string(llvm::Function* generated_code);
};

}
