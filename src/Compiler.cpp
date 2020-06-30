#include <san/Compiler.hpp>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/Target/TargetMachine.h>

#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include <iostream>

std::vector<std::string> San::Compiler::generate_objects(const std::string &os, const std::string &arch, const llvm::PassBuilder::OptimizationLevel &optimization_level)
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string error;
    std::string target_triple = this->module->getTargetTriple();
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(target_triple, error);

    if (!target)
    {
        std::cerr << error << std::endl;
        return {};
    }

    std::string cpu = "generic";
    std::string features = "";

    llvm::TargetOptions target_options;
    llvm::Optional<llvm::Reloc::Model> rm = llvm::Optional<llvm::Reloc::Model>();
    llvm::TargetMachine *target_machine = target->createTargetMachine(target_triple, cpu, features, target_options, rm);

    this->module->setDataLayout(target_machine->createDataLayout());
    this->module->setTargetTriple(target_triple);

    std::string output_path = std::tmpnam(nullptr);
    std::error_code error_code;
    llvm::raw_fd_ostream dest(output_path, error_code, llvm::sys::fs::OF_None);

    if (error_code)
    {
        llvm::errs() << "Could not open file: " << error_code.message();
        return {};
    }

    llvm::PassBuilder builder;
    llvm::LoopAnalysisManager loop_analisys_manager(false);
    llvm::FunctionAnalysisManager function_analisys_manager(false);
    llvm::CGSCCAnalysisManager CGSCC_analisys_manager(false);
    llvm::ModuleAnalysisManager module_analisys_manager(false);

    builder.registerModuleAnalyses(module_analisys_manager);
    builder.registerCGSCCAnalyses(CGSCC_analisys_manager);
    builder.registerFunctionAnalyses(function_analisys_manager);
    builder.registerLoopAnalyses(loop_analisys_manager);
    builder.crossRegisterProxies(loop_analisys_manager, function_analisys_manager, CGSCC_analisys_manager, module_analisys_manager);

    llvm::ModulePassManager module_pass_manager = builder.buildModuleOptimizationPipeline(optimization_level);
    module_pass_manager.run(*module, module_analisys_manager);

    llvm::legacy::PassManager pass;

    pass.add(llvm::createInstructionCombiningPass());
    pass.add(llvm::createReassociatePass());
    pass.add(llvm::createGVNPass());
    pass.add(llvm::createCFGSimplificationPass());

    llvm::CodeGenFileType file_type = llvm::CGFT_ObjectFile;

    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type))
    {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return {};
    }

    pass.run(*this->module);
    dest.flush();

    return {output_path};
}
