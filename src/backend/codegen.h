#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "./../trees.fwd.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

class GeneratorVisitor{
    std::unique_ptr<llvm::LLVMContext> ctx;
    std::unique_ptr<llvm::IRBuilder<>> bldr;
    std::unique_ptr<llvm::Module> mdl;
    std::map<std::string, llvm::AllocaInst*> values;
    std::unique_ptr<llvm::FunctionPassManager> fpm;
    std::unique_ptr<llvm::LoopAnalysisManager> lam;
    std::unique_ptr<llvm::FunctionAnalysisManager> fam;
    std::unique_ptr<llvm::CGSCCAnalysisManager> cgam;
    std::unique_ptr<llvm::ModuleAnalysisManager> mam;
    std::unique_ptr<llvm::PassInstrumentationCallbacks> pic;
    std::unique_ptr<llvm::StandardInstrumentations> si;
    std::vector<IntDecAST*> globalVars;

public:
    llvm::Value* generate(IntDecAST* ast);
    llvm::Value* generate(CallAST* ast);
    llvm::Function* generate(FunDecAST* ast);
    llvm::Function* generate(FunDefAST* ast);
    llvm::Value* generate(ReturnAST* ast);
    llvm::Value* generate(IntAST* ast);
    llvm::Value* generate(IdentAST* ast);
    llvm::Value* generate(BinExprAST* ast);
    llvm::Value* generate(AssignAST* ast);
    llvm::Value* generate(IfElseAST* ast);

    GeneratorVisitor(){
        auto TargetTriple = llvm::sys::getDefaultTargetTriple();
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
        if(!Target) {
            llvm::errs() << Error;
            return;
        }
        auto CPU = "generic";
        auto Features = "";
        llvm::TargetOptions opt;
        auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);
        ctx = std::make_unique<llvm::LLVMContext>();
        mdl = std::make_unique<llvm::Module>("my cool compiler", *ctx);
        mdl->setDataLayout(TargetMachine->createDataLayout());
        mdl->setTargetTriple(TargetTriple);
        bldr = std::make_unique<llvm::IRBuilder<>>(*ctx);

        fpm = std::make_unique<llvm::FunctionPassManager>();
        lam = std::make_unique<llvm::LoopAnalysisManager>();
        fam = std::make_unique<llvm::FunctionAnalysisManager>();
        cgam = std::make_unique<llvm::CGSCCAnalysisManager>();
        mam = std::make_unique<llvm::ModuleAnalysisManager>();
        pic = std::make_unique<llvm::PassInstrumentationCallbacks>();
        si = std::make_unique<llvm::StandardInstrumentations>(*ctx, true);
        si->registerCallbacks(*pic, mam.get());

        // Add transformation passes
        fpm->addPass(llvm::InstCombinePass()); // Peephole optimizations
        fpm->addPass(llvm::ReassociatePass()); // Reassociate commutative expressions
        fpm->addPass(llvm::GVNPass()); // Global value numbering pass, CSE & GCP
        fpm->addPass(llvm::SimplifyCFGPass()); // Delete unreachable code, clean up CFG
        llvm::PassBuilder pb;
        pb.registerModuleAnalyses(*mam);
        pb.registerFunctionAnalyses(*fam);
        pb.crossRegisterProxies(*lam, *fam, *cgam, *mam);
    }

    void finish(){
        auto TargetTriple = llvm::sys::getDefaultTargetTriple();
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
        if(!Target) {
            llvm::errs() << Error;
            return;
        }
        auto CPU = "generic";
        auto Features = "";
        llvm::TargetOptions opt;
        auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_);

        auto Filename = "output.o";
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        if(EC){
            llvm::errs() << "Could not open file: " << EC.message();
            return;
        }

        llvm::legacy::PassManager pass;
        auto FileType = llvm::CodeGenFileType::ObjectFile;

        if(TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)){
            llvm::errs() << "TargetMachine can't emit a file of this type";
            return;
        }

        pass.run(*mdl);
        dest.flush();

        llvm::outs() << "Wrote " << Filename << "\n";
    }
};

