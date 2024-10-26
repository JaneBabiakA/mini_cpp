#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "trees.fwd.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

class GeneratorVisitor{
    std::unique_ptr<llvm::LLVMContext> ctx;
    std::unique_ptr<llvm::IRBuilder<>> bldr;
    std::unique_ptr<llvm::Module> mdl;
    std::map<std::string, llvm::Value*> values;
public:
llvm::Value* generate(IntDecAST *ast);
llvm::Value* generate(CallAST *ast);
llvm::Function* generate(FunDecAST *ast);
llvm::Function* generate(FunDefAST *ast);
llvm::Value* generate(ReturnAST *ast);
llvm::Value* generate(IntAST *ast);
llvm::Value* generate(IdentAST *ast);
llvm::Value* generate(BinExprAST *ast);
llvm::Value* generate(AssignAST *ast);

GeneratorVisitor(){
    ctx = std::make_unique<llvm::LLVMContext>();
    mdl = std::make_unique<llvm::Module>("my cool jit", *ctx);
    bldr = std::make_unique<llvm::IRBuilder<>>(*ctx);
}
};

