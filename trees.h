#pragma once
#include <string>
#include <memory>
#include <vector>
//#include <llvm-18/llvm/IR/Value.h>
//#include <llvm-18/llvm/IR/Function.h>
#include "lexer.h" //TODO: phase out
#include "codegen.fwd.h"
#include "codegen.h"

#include <llvm-18/llvm/IR/DerivedTypes.h>
#include <llvm-14/llvm/IR/Verifier.h>
#include <llvm-14/llvm/IR/IRBuilder.h>
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

class ExpressionAST{
public:
    ExpressionAST() {

    }
    virtual llvm::Value* accept(GeneratorVisitor *visitor){
        return nullptr;
    };
};

class FunctionAST{
public:
    FunctionAST(){

    }
    virtual llvm::Function* accept(GeneratorVisitor *visitor){
        return nullptr;
    };
};

class IdentAST : public ExpressionAST{
public:
    IdentAST(std::string name) : m_name(std::move(name)) {}
    std::string m_name;
    llvm::Value* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }
};

class IntAST:  public ExpressionAST{
public:
    IntAST(std::string value) : m_value(value) {}
    llvm::Value* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }

    std::string m_value;
};

class IntDecAST: public ExpressionAST{ //TODO: add in scoping?
public:
    IntDecAST(IdentAST ident, std::unique_ptr<ExpressionAST> value): m_ident(std::move(ident)), m_value(std::move(value)) {}
    IdentAST m_ident;
    std::unique_ptr<ExpressionAST> m_value; //Should be IntAST or CallAST
    llvm::Value* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }
};

class BinExprAST : public ExpressionAST{
public:
    BinExprAST(std::unique_ptr<Token> op, std::unique_ptr<ExpressionAST> LHS, std::unique_ptr<ExpressionAST> RHS) : m_op(std::move(op)), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}
    llvm::Value* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }
    std::unique_ptr<Token> m_op;
    std::unique_ptr<ExpressionAST> m_LHS;
    std::unique_ptr<ExpressionAST> m_RHS;
};

class CallAST : public ExpressionAST{
public:
    CallAST(std::string call, std::vector<std::unique_ptr<ExpressionAST>> args) : m_call(std::move(call)), m_args(std::move(args)) {}
    llvm::Value* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }
    std::vector<std::unique_ptr<ExpressionAST>> m_args;
    std::string m_call;
};

class FunDecAST: public FunctionAST{ //TODO: create getter for name
public:
    FunDecAST(std::string type, std::string name, std::vector<std::unique_ptr<IntDecAST>> args) : m_type(std::move(type)), m_name(std::move(name)), m_args(std::move(args)) {}
    std::string m_name;
    llvm::Function* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }
    std::string m_type;
    std::vector<std::unique_ptr<IntDecAST>> m_args;
};

class FunDefAST: public FunctionAST{
public:
    FunDefAST(FunDecAST dec, std::vector<std::unique_ptr<ExpressionAST>> body) : m_dec(std::move(dec)), m_body(std::move(body)) {}
    llvm::Function* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }
    FunDecAST m_dec;
    std::vector<std::unique_ptr<ExpressionAST>> m_body;
};

class AssignAST: public ExpressionAST{
public:
    AssignAST(IdentAST ident, std::unique_ptr<ExpressionAST> value): m_ident(std::move(ident)), m_value(std::move(value)){};
    llvm::Value* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }
    IdentAST m_ident;
    std::unique_ptr<ExpressionAST> m_value;
};

class ReturnAST: public ExpressionAST{
public:
    ReturnAST(std::unique_ptr<ExpressionAST> value): m_value(std::move(value)){};
    llvm::Value* accept(GeneratorVisitor *visitor){
        return visitor->generate(this);
    }
    std::unique_ptr<ExpressionAST> m_value;
};
