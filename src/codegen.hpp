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

using CodegenContentType = std::variant<
    llvm::Value*,
    llvm::Function*,
    std::monostate>;

class CodeGeneratorLLVM
{
    std::unique_ptr<llvm::LLVMContext> context{std::make_unique<llvm::LLVMContext>()};
    std::unique_ptr<llvm::IRBuilder<>> builder{std::make_unique<llvm::IRBuilder<>>(*context)};
    std::unique_ptr<llvm::Module> module{std::make_unique<llvm::Module>("kccani_jit", *context)};
    std::map<std::string, llvm::Value*> named_values;

public:
    CodegenContentType operator()(std::unique_ptr<ExprAST>&& ast);
    CodegenContentType operator()(std::unique_ptr<FunctionAST>&& ast);
    CodegenContentType operator()(std::unique_ptr<FunctionPrototypeAST>&& ast);
    CodegenContentType operator()(std::monostate&& ast);

    void print() const;
    std::string to_string() const;
    static void print(CodegenContentType generated_code);
    static std::string to_string(CodegenContentType generated_code);
};

}
