#include "evaluator.hpp"

namespace kccani
{

JITEvaluatorLLVM::JITEvaluatorLLVM()
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    this->jit = this->exit_on_error(KaleidoscopeJIT::create());
}

void JITEvaluatorLLVM::reinitialize_module()
{
    this->context = std::make_unique<llvm::LLVMContext>();
    this->module = std::make_unique<llvm::Module>("my cool jit", *this->context);
    this->module->setDataLayout(jit->get_data_layout());
    this->builder = std::make_unique<llvm::IRBuilder<>>(*this->context);
    this->fpm = std::make_unique<llvm::legacy::FunctionPassManager>(this->module.get());

    this->fpm->add(llvm::createInstructionCombiningPass());  // Simple "peephole" & bit-twiddling
    this->fpm->add(llvm::createReassociatePass());  // Reassociate expressions
    this->fpm->add(llvm::createGVNPass());  // Eliminate common sub-expressions.
    this->fpm->add(llvm::createCFGSimplificationPass());  // Simplify control flow graph (e.g. unreachable code)

    this->fpm->doInitialization();
}

llvm::Function* JITEvaluatorLLVM::get_function(const std::string& name)
{
    if (auto *function = this->module->getFunction(name))
        return function;

    auto fn_map_iterator = this->function_protos.find(name);
    if (fn_map_iterator != this->function_protos.end())
        return (*this)(std::move(fn_map_iterator->second));
    return nullptr;
}

llvm::Function* JITEvaluatorLLVM::operator()(std::unique_ptr<ExprAST>&& ast)
{
    auto generated_code = (*this)(std::move(ast));
    auto resource_tracker = this->jit->get_main_jit_dylib().createResourceTracker();

    this->exit_on_error(
        this->jit->add_module(
            llvm::orc::ThreadSafeModule(std::move(this->module), std::move(this->context)),
            resource_tracker)
    );
    this->reinitialize_module();

    auto expr_symbol = this->exit_on_error(this->jit->lookup("__anon_expr"));

    double (*resultant_value_generator)() = expr_symbol.getAddress().toPtr<double (*)()>();
    this->last_value = resultant_value_generator();

    this->exit_on_error(resource_tracker->remove());
    return generated_code;
}

llvm::Function* JITEvaluatorLLVM::operator()(std::unique_ptr<FunctionAST>&& ast)
{
    auto prototype_copy = std::unique_ptr<FunctionPrototypeAST>(ast->prototype.get());
    this->function_protos[prototype_copy->name] = std::move(prototype_copy);

    auto generated_code = CodeGeneratorLLVM::operator()(std::move(ast));
    this->fpm->run(*generated_code);

    this->exit_on_error(
        this->jit->add_module(
            llvm::orc::ThreadSafeModule(std::move(this->module), std::move(this->context)))
    );
    this->reinitialize_module();

    return generated_code;
}

llvm::Function* JITEvaluatorLLVM::operator()(std::unique_ptr<FunctionPrototypeAST>&& ast)
{
    this->function_protos[ast->name] = std::move(ast);
    return CodeGeneratorLLVM::operator()(std::move(ast));
}

llvm::Function* JITEvaluatorLLVM::operator()(std::monostate&& ast)
{
    return CodeGeneratorLLVM::operator()(std::move(ast));
}

std::optional<double> JITEvaluatorLLVM::get()
{
    this->last_value = std::nullopt;
    return this->last_value;
}

std::optional<double> JITEvaluatorLLVM::peek()
{
    return this->last_value;
}

// Copied and adapted from:
// https://github.com/llvm/llvm-project/blob/main/llvm/examples/Kaleidoscope/include/KaleidoscopeJIT.h

KaleidoscopeJIT::KaleidoscopeJIT(
    std::unique_ptr<llvm::orc::ExecutionSession> execution_session,
    llvm::orc::JITTargetMachineBuilder jit_builder,
    llvm::DataLayout data_layout
) : session(std::move(execution_session)),
    data_layout(std::move(data_layout)),
    mangle(*this->session, this->data_layout),
    object_layer(
        *this->session,
        []() { return std::make_unique<llvm::SectionMemoryManager>(); }
    ),
    compile_layer(
        *this->session,
        object_layer,
        std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(jit_builder))
    ),
    main_jd(this->session->createBareJITDylib("<main>"))
{
    main_jd.addGenerator(
        cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
            data_layout.getGlobalPrefix())));
    if (jit_builder.getTargetTriple().isOSBinFormatCOFF())
    {
        object_layer.setOverrideObjectFlagsWithResponsibilityFlags(true);
        object_layer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
}

KaleidoscopeJIT::~KaleidoscopeJIT()
{
    if (auto error = session->endSession())
        session->reportError(std::move(error));
}

llvm::Expected<std::unique_ptr<KaleidoscopeJIT>> KaleidoscopeJIT::create()
{
    auto exec_proc_control = llvm::orc::SelfExecutorProcessControl::Create();
    if (!exec_proc_control)
        return exec_proc_control.takeError();

    auto execution_session = std::make_unique<llvm::orc::ExecutionSession>(
        std::move(*exec_proc_control));

    llvm::orc::JITTargetMachineBuilder jit_builder(
        execution_session->getExecutorProcessControl().getTargetTriple());

    auto data_layout = jit_builder.getDefaultDataLayoutForTarget();
    if (!data_layout)
        return data_layout.takeError();

    return std::make_unique<KaleidoscopeJIT>(
        std::move(execution_session),
        std::move(jit_builder),
        std::move(*data_layout)
    );
}

const llvm::DataLayout &KaleidoscopeJIT::get_data_layout() const
{
    return data_layout;
}

llvm::orc::JITDylib &KaleidoscopeJIT::get_main_jit_dylib()
{
    return main_jd;
}

llvm::Error KaleidoscopeJIT::add_module(
    llvm::orc::ThreadSafeModule thread_safe_module,
    llvm::orc::ResourceTrackerSP resource_tracker
)
{
    if (!resource_tracker)
        resource_tracker = main_jd.getDefaultResourceTracker();
    return compile_layer.add(resource_tracker, std::move(thread_safe_module));
}

llvm::Expected<llvm::orc::ExecutorSymbolDef> KaleidoscopeJIT::lookup(llvm::StringRef name) {
    return session->lookup({&main_jd}, mangle(name.str()));
}

}
