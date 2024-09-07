#pragma once
#include<string>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include "lexer.h"

class AST{
public:
    virtual ~AST() = default;
};

class IdentAST : public AST{
    std::string m_name;
public: IdentAST(std::string name) : m_name(std::move(name)) {}
};

class IntAST:  public AST{
    std::string m_value;
public: IntAST(std::string value) : m_value(value) {}
};

class IntDecAST: public AST{
    IdentAST m_ident;
    IntAST m_value;
public: IntDecAST(IdentAST ident, IntAST value): m_ident(std::move(ident)), m_value(std::move(value)) {}
};

class BinExprAST : public AST{
    Token m_op;
    std::unique_ptr<AST> m_LHS, m_RHS;
public:
    BinExprAST(Token op, std::unique_ptr<AST> LHS, std::unique_ptr<AST> RHS) : m_op(std::move(op)), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}
};

class CallAST : public AST{
    std::string m_call;
    std::vector<std::unique_ptr<AST>> m_args;
public:
    CallAST(std::string call, std::vector<std::unique_ptr<AST>> args) : m_call(std::move(call)), m_args(std::move(args)) {}
};

class FunDecAST{ //TODO: add a return type field?
    std::string m_type;
    std::string m_name;
    std::vector<std::string> m_args;
public:
    FunDecAST(std::string type, std::string name, std::vector<std::string> args) : m_type(std::move(type)), m_name(std::move(name)), m_args(std::move(args)) {}
    [[nodiscard]] const std::string &getName() const { return m_name; }
};

class FunDefAST{
    std::unique_ptr<FunDecAST> m_dec;
    std::unique_ptr<AST> m_body;
public:
    FunDefAST(std::unique_ptr<FunDecAST> dec, std::unique_ptr<AST> body) : m_dec(std::move(dec)), m_body(std::move(body)) {}
};

class Parser{
    int m_index = 0;
    static inline std::map<TokenTypes, int> m_precedence = {{TokenTypes::Token_Plus, 1}, {TokenTypes::Token_Multiply, 2}};
    std::vector<Token> m_tokens;

    std::optional<Token> fetchToken(){ //TODO: figure out how to deal with end of file/end of stream
        if(m_index < m_tokens.size()){
            return m_tokens[m_index];
        }
        return {};
    }
    std::unique_ptr<AST> LogError(char *error){
        fprintf(stderr, "Error: %s\n", error);
        return nullptr;
    }
    std::unique_ptr<IntAST> parseInt(Token token){
        auto result = std::make_unique<IntAST>(token.value.value());
        m_index++;
        return result;
    }
    std::unique_ptr<AST> parseBrac(){
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
    std::unique_ptr<AST> parseIdent(Token token){
        m_index++;
        if(fetchToken().value().type != TokenTypes::Token_OpenR){
            return std::make_unique<IdentAST>(token.value.value());
        }
        m_index++;
        std::vector<std::unique_ptr<AST>> args;
        while(fetchToken().value().type != TokenTypes::Token_CloseR){
            if(auto arg = parseExpr()){
                args.push_back(std::move(arg));
            }
            else{
                return nullptr;
            }
            if(fetchToken().value().type != TokenTypes::Token_CloseR && fetchToken().value().type != TokenTypes::Token_Comma){
                return LogError("Expected ')' or ',' in argument list");
            }
            m_index++;
        }
        m_index++;
        return std::make_unique<CallAST>(token.value.value(), std::move(args));
    }
    std::unique_ptr<AST> parseExpr(){
        switch(fetchToken().value().type){
            case TokenTypes::Token_Ident:
                return parseIdent(fetchToken().value()); //TODO: rename to 'description' possibly? this is quite confusing
            case TokenTypes::Token_Int:
                return parseInt(fetchToken().value());
            case TokenTypes::Token_OpenR:
                return parseBrac();
            default:
                return nullptr;
        }
    }

    int getPrecendence(){
        int prec = m_precedence[fetchToken().value().type];
        if(prec == 0){
            return -1;
        }
        return prec;
    }
    std::unique_ptr<AST> parseBinRHS(int prevPrec, std::unique_ptr<AST> LHS){
        while(true){
            int currPrec = getPrecendence();
            if(currPrec < prevPrec){
                return LHS;
            }
            Token op = fetchToken().value();
            m_index++;
            auto RHS = parseExpr();
            if(!RHS){
                return nullptr;
            }
            int nextPrec = getPrecendence();
            if(currPrec < nextPrec){
                RHS = parseBinRHS(currPrec+1, std::move(RHS));
                if(!RHS){
                    return nullptr;
                }
            }
            LHS = std::make_unique<BinExprAST>(op, std::move(LHS), std::move(RHS));
        }
    }

public:
    AST parse(){


    }

    explicit Parser(std::vector<Token> tokens){
        m_tokens = std::move(tokens);
    }

    std::unique_ptr<AST> parseBinExpr() {
        auto LHS = parseExpr();
        if (!LHS) {
            return nullptr;
        }
        return parseBinRHS(0, std::move(LHS));
    }
};
