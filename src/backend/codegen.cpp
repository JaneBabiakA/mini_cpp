#include "./../frontend/lexer.h"
#include "./../trees.h"
#include "./../frontend/parser.h"
#include "codegen.h"
#include <string>
#include <vector>
#include <memory>
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/GlobalVariable.h"
#include <iostream>

llvm::Value* GeneratorVisitor::generate(IntDecAST* ast){
    llvm::Type* intType = llvm::Type::getInt32Ty(*ctx);
    // Check variable scope
    if(ast->m_isGlobal){ 
        llvm::Constant* init = llvm::ConstantInt::get(intType, 0);
        auto* globalVar =  new llvm::GlobalVariable(*mdl, intType, false, llvm::GlobalValue::ExternalLinkage, init, ast->m_ident.m_name);
        globalVars.push_back(ast);
        return globalVar;
    }
    llvm::AllocaInst* alloc = bldr->CreateAlloca(intType, nullptr, ast->m_ident.m_name);
    if(ast->m_value){
        bldr->CreateStore(ast->m_value->accept(this), alloc);
    }
    values[ast->m_ident.m_name] = alloc;
    return nullptr;
}

llvm::Value* GeneratorVisitor::generate(CallAST* ast){
    llvm::Function *call = mdl->getFunction(ast->m_call);
    if(!call){
        return nullptr;
    }
    if(call->arg_size() != ast->m_args.size()){ // Check for correct number of arguments
        return nullptr;
    }
    std::vector<llvm::Value *> argsIR;
    for(auto & m_arg : ast->m_args){
        argsIR.push_back(m_arg->accept(this));
        if(!argsIR.back()){
            return nullptr;
        }
    }
    return bldr->CreateCall(call, argsIR, "calltmp");
}

llvm::Function* GeneratorVisitor::generate(FunDecAST* ast){
    std::vector<llvm::Type *> arg_types(ast->m_args.size(), llvm::Type::getInt32Ty(*ctx));
    llvm::FunctionType *fun_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx), arg_types, false);

    llvm::Function *function = llvm::Function::Create(fun_type, llvm::Function::ExternalLinkage, ast->m_name, mdl.get());

    int i = 0;
    for(auto &arg : function->args()){
        arg.setName(ast->m_args[i++]->m_ident.m_name);
    }

    return function;
}

llvm::Function* GeneratorVisitor::generate(FunDefAST* ast){
    llvm::Function *function = mdl->getFunction(ast->m_dec->m_name);
    if(!function){
        function = ast->m_dec->accept(this);
    }
    if(!function->empty()){
        return nullptr;
        //return (llvm::Function*)LogErrorV("Function cannot be redefined.")
    }
    llvm::BasicBlock *block = llvm::BasicBlock::Create(*ctx, "entry", function);
    bldr->SetInsertPoint(block);
    values.clear();

    // Initialize all uninitialized global variables
    for(auto globalVar: globalVars){
        llvm::GlobalVariable *test = mdl->getGlobalVariable(globalVar->m_ident.m_name);
        bldr->CreateStore(globalVar->m_value->accept(this), test);
    }
    // TODO: delete all global vars
    globalVars.clear();

    // Allocate and initialize all arguments
    for(auto &arg : function->args()){
        llvm::AllocaInst* alloc = bldr->CreateAlloca(llvm::Type::getInt32Ty(*ctx), nullptr, arg.getName());
        bldr->CreateStore(&arg, alloc);
        values[std::string(arg.getName())] = alloc;
    }
    for(auto &ast : ast->m_body){
        ast->accept(this);
    }
    llvm::verifyFunction(*function);
    fpm->run(*function, *fam);
    return function;
}

llvm::Value* GeneratorVisitor::generate(ReturnAST* ast){
    bldr->CreateRet(ast->m_value->accept(this));
    return nullptr;
}

llvm::Value* GeneratorVisitor::generate(IntAST* ast){
    return llvm::ConstantInt::get(*ctx, llvm::APInt(32, stoi(ast->m_value)));
}

llvm::Value* GeneratorVisitor::generate(IdentAST* ast){
    if(llvm::GlobalVariable *globalVar = mdl->getGlobalVariable(ast->m_name)){ // Check if name matches a global variable
        return bldr->CreateLoad(globalVar->getValueType(), globalVar, ast->m_name);
    }
    else if(llvm::AllocaInst *alloc = values[ast->m_name]){ // Check if name matches a local variable
        return bldr->CreateLoad(alloc->getAllocatedType(), alloc, ast->m_name);
    }
    else {
        return nullptr;
    }
}

llvm::Value* GeneratorVisitor::generate(BinExprAST* ast) {
    llvm::Value * L = ast->m_LHS->accept(this);
    llvm::Value * R = ast->m_RHS->accept(this);
    if (!L || !R) {
        return nullptr;
    }
    switch (ast->m_op->type) {
        case TokenTypes::Token_Plus: {
            return bldr->CreateAdd(L, R, "addtmp");
        }
        case TokenTypes::Token_Minus: {
            return bldr->CreateSub(L, R, "subtmp");
        }
        case TokenTypes::Token_Multiply: {
            return bldr->CreateMul(L, R, "multmp");
        }
        case TokenTypes::Token_Divide: {
            return bldr->CreateSDiv(L, R, "divtmp"); // Create a signed division
        }
        default: {
            return nullptr;
        }
    }
}

llvm::Value* GeneratorVisitor::generate(AssignAST* ast) {
    llvm::AllocaInst* alloc = values[ast->m_ident.m_name];
    if(!alloc){
        return nullptr;
    }
    llvm::Value *ident = bldr->CreateLoad(alloc->getAllocatedType(), alloc, ast->m_ident.m_name);
    bldr->CreateStore(ast->m_value->accept(this), alloc);
    return nullptr;
}

llvm::Value* GeneratorVisitor::generate(IfElseAST* ast){
    llvm::Value* cond = ast->m_condition->accept(this);
    if(!cond){
        return nullptr;
    }
    // Create boolean from condition
    cond = bldr->CreateICmpNE(cond, llvm::ConstantInt::get(*ctx, llvm::APInt(32, 0)), "ifcond");
    // Create blocks for if/else cases and merge
    llvm::Function *function = bldr->GetInsertBlock()->getParent();
    llvm::BasicBlock *ifBB = llvm::BasicBlock::Create(*ctx, "if", function);
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(*ctx, "else");
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*ctx, "merge");
    bldr->CreateCondBr(cond, ifBB, elseBB);
    // Insert if statement body
    bldr->SetInsertPoint(ifBB);
    for(auto &line : ast->m_body){
        line->accept(this);
    }
    // Create branch back to merge block
    bldr->CreateBr(mergeBB);
    // Reset insert block to current (in case of nested if statement)
    ifBB = bldr->GetInsertBlock();
    // Add else block to end of function and insert body into it
    function->insert(function->end(), elseBB);
    bldr->SetInsertPoint(elseBB);
    for(auto &line : ast->m_else){
        line->accept(this);
    }
    // Create branch back to merge block
    bldr->CreateBr(mergeBB);
    // Reset insert block to current
    elseBB = bldr->GetInsertBlock();
    function->insert(function->end(), mergeBB);
    bldr->SetInsertPoint(mergeBB);
    return nullptr;
}
