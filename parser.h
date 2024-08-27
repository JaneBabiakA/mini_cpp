#pragma once
#include<string>
#include <memory>
#include <utility>
#include <vector>
#include "lexer.h"

class ExprAST{
public:
    virtual ~ExprAST() = default;
};

class IntExprAST:  public ExprAST{
    int m_value;
public: IntExprAST(int value) : m_value(value) {}
};

class VarExprAST : public ExprAST{
    std::string m_name;
public: VarExprAST(std::string name) : m_name(std::move(name)) {}
};

class BinaryExprAST : public ExprAST{
    char m_op;
    std::unique_ptr<ExprAST> m_LHS, m_RHS;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) : m_op(op), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}
};

class CallExprAST : public ExprAST{
    std::string m_call;
    std::vector<std::unique_ptr<ExprAST>> m_args;
public:
    CallExprAST(std::string call, std::vector<std::unique_ptr<ExprAST>> args) : m_call(std::move(call)), m_args(std::move(args)) {}
};

class DeclarationAST{ //TODO: add a return type field?
    std::string m_type;
    std::string m_name;
    std::vector<std::string> m_args;
public:
    DeclarationAST(std::string type, std::string name, std::vector<std::string> args) : m_type(std::move(type)), m_name(std::move(name)), m_args(std::move(args)) {}
    [[nodiscard]] const std::string &getName() const { return m_name; }
};

class DefinitionAST{
    std::unique_ptr<DeclarationAST> m_dec;
    std::unique_ptr<ExprAST> m_body;
public:
    DefinitionAST(std::unique_ptr<DeclarationAST> dec, std::unique_ptr<ExprAST> body) : m_dec(std::move(dec)), m_body(std::move(body)) {}
};

class Parser{
    int m_index = 0;
    std::vector<Token> m_tokens;
    std::optional<Token> fetchToken(){
        if(m_index < m_tokens.size()){
            return m_tokens[m_index];
        }
        return {};
    }
    std::unique_ptr<ExprAST> LogError(char *error){
        fprintf(stderr, "Error: %s\n", error);
        return nullptr;
    }
    std::unique_ptr<IntExprAST> parseIntExpr(int value){
        auto result = std::make_unique<IntExprAST>(value);
        m_index++;
        return result;
    }
    std::unique_ptr<ExprAST> parseBracExpr(){
        m_index++;
        auto content = parseExpr();
        if(!content){ //Check for empty brackets case
            return nullptr;
        }
        if(fetchToken().has_value() && fetchToken().value().type != TokenTypes::Token_CloseR){
            return LogError("')' expected");
        }
        m_index++;
        return content;
    }
    std::unique_ptr<ExprAST> parseExpr(){
        return nullptr;
    }
public:
    ExprAST parse(){


    }
};
