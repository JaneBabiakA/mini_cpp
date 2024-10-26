#include "lexer.h"
#include "trees.h"
#include "parser.h"
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

llvm::Value *GeneratorVisitor::generate(IntDecAST *ast){
    return nullptr;
}

llvm::Value *GeneratorVisitor::generate(CallAST *ast){
    llvm::Function *call = mdl->getFunction(ast->m_call);
    if(!call){
        return nullptr; //something about unknown function
    }
    if(call->arg_size() != ast->m_args.size()){ //Check for correct number of arguments
        return nullptr; //error
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

llvm::Function* GeneratorVisitor::generate(FunDecAST *ast){ //TODO: Edit this once I add in function types
    std::vector<llvm::Type *> arg_types(ast->m_args.size(), llvm::Type::getInt32Ty(*ctx));
    llvm::FunctionType *fun_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx), arg_types, false);
    llvm::Function *function = llvm::Function::Create(fun_type, llvm::Function::ExternalLinkage, ast->m_name, mdl.get());
//    int i = 0;
//    for(auto &arg : function->args()){
//        arg.setName(ast->m_args[i++]->m_ident.m_name);
//    }
    return function;
}

llvm::Function *GeneratorVisitor::generate(FunDefAST *ast){
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
    values.clear(); //TODO: check on this later
    for(auto &arg : function->args()){
        values[std::string(arg.getName())] = &arg;
    }
    for(auto &ast : ast->m_body){
        ast->accept(this);
    }
    llvm::verifyFunction(*function);
    return function;
}

llvm::Value *GeneratorVisitor::generate(ReturnAST *ast){
    bldr->CreateRet(ast->m_value->accept(this));
    return nullptr;
}

llvm::Value *GeneratorVisitor::generate(IntAST *ast){
    return llvm::ConstantInt::get(*ctx, llvm::APInt(32, stoi(ast->m_value)));
}

llvm::Value *GeneratorVisitor::generate(IdentAST *ast){
    llvm::Value *v = values[ast->m_name]; //Look up variable
    if(!v){
        //return error("Unknown variable name")
    }
    return v;
}

llvm::Value *GeneratorVisitor::generate(BinExprAST *ast) {
    llvm::Value * L = ast->m_LHS->accept(this);
    llvm::Value * R = ast->m_RHS->accept(this);
    if (!L || !R) {
        //return an error
        return nullptr;
    }
    switch (ast->m_op->type) {
        case TokenTypes::Token_Plus: {
            return bldr->CreateFAdd(L, R, "addtmp");
        }
        case TokenTypes::Token_Multiply: {
            return bldr->CreateFMul(L, R, "multmp");
        }
        default: {
            return nullptr;
        }
    }
}

llvm::Value *GeneratorVisitor::generate(AssignAST *ast) {
    return nullptr;
}
