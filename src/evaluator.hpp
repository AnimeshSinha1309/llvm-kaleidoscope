#pragma once

#include <iostream>
#include <memory>

#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include "codegen.hpp"

namespace kccani
{

class KaleidoscopeJIT {

private:
    std::unique_ptr<llvm::orc::ExecutionSession> session;
    llvm::DataLayout data_layout;
    llvm::orc::MangleAndInterner mangle;
    llvm::orc::RTDyldObjectLinkingLayer object_layer;
    llvm::orc::IRCompileLayer compile_layer;
    llvm::orc::JITDylib &main_jd;

public:
    KaleidoscopeJIT(
        std::unique_ptr<llvm::orc::ExecutionSession> execution_session,
        llvm::orc::JITTargetMachineBuilder jit_builder,
        llvm::DataLayout data_layout
    );
    ~KaleidoscopeJIT();

    static llvm::Expected<std::unique_ptr<KaleidoscopeJIT>> create();
    const llvm::DataLayout &get_data_layout() const;
    llvm::orc::JITDylib &get_main_jit_dylib();
    llvm::Error add_module(
        llvm::orc::ThreadSafeModule thread_safe_module,
        llvm::orc::ResourceTrackerSP resource_tracker = nullptr
    );
    llvm::Expected<llvm::orc::ExecutorSymbolDef> lookup(llvm::StringRef Name);
};

class JITEvaluatorLLVM : CodeGeneratorLLVM
{
    std::unique_ptr<KaleidoscopeJIT> jit;
    std::unique_ptr<llvm::legacy::FunctionPassManager> fpm;
    std::map<std::string, std::unique_ptr<FunctionPrototypeAST>> function_protos;
    static llvm::ExitOnError exit_on_error;

private:
    void reinitialize_module();
    llvm::Function* get_function(const std::string& name) override;
    std::optional<double> last_value = std::nullopt;

public:
    JITEvaluatorLLVM();

    // Handing top-level expressions
    virtual llvm::Function* operator()(std::unique_ptr<ExprAST>&& ast) override;
    // Handling function definitions
    virtual llvm::Function* operator()(std::unique_ptr<FunctionAST>&& ast) override;
    // Handling external linkage
    virtual llvm::Function* operator()(std::unique_ptr<FunctionPrototypeAST>&& ast) override;
    // Handling statements with parsing errors in them
    virtual llvm::Function* operator()(std::monostate&& ast) override;

    virtual std::optional<double> get();
    virtual std::optional<double> peek();
};

}
