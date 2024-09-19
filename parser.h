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
    std::unique_ptr<AST> m_value; //Should be IntAST or CallAST
public: IntDecAST(IdentAST ident, std::unique_ptr<AST> value): m_ident(std::move(ident)), m_value(std::move(value)) {}
};

class BinExprAST : public AST{
    std::unique_ptr<Token> m_op;
    std::unique_ptr<AST> m_LHS, m_RHS;
public:
    BinExprAST(std::unique_ptr<Token> op, std::unique_ptr<AST> LHS, std::unique_ptr<AST> RHS) : m_op(std::move(op)), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}
};

class CallAST : public AST{
    std::string m_call;
    std::vector<std::unique_ptr<AST>> m_args;
public:
    CallAST(std::string call, std::vector<std::unique_ptr<AST>> args) : m_call(std::move(call)), m_args(std::move(args)) {}
};

class FunDecAST: public AST{
    std::string m_type;
    std::string m_name;
    std::vector<std::unique_ptr<AST>> m_args;
public:
    FunDecAST(std::string type, std::string name, std::vector<std::unique_ptr<AST>> args) : m_type(std::move(type)), m_name(std::move(name)), m_args(std::move(args)) {}
    [[nodiscard]] const std::string &getName() const { return m_name; }
};

class FunDefAST: public AST{
    std::unique_ptr<FunDecAST> m_dec;
    std::vector<std::unique_ptr<AST>> m_body;
public:
    FunDefAST(std::unique_ptr<FunDecAST> dec, std::vector<std::unique_ptr<AST>> body) : m_dec(std::move(dec)), m_body(std::move(body)) {}
};

class Parser{
    int m_index = 0;
    static inline std::map<TokenTypes, int> m_precedence = {{TokenTypes::Token_Plus, 1}, {TokenTypes::Token_Multiply, 2}};
    std::vector<Token> m_tokens;

    std::unique_ptr<Token> fetchToken(){ //TODO: figure out how to deal with end of file/end of stream
        if(m_index < m_tokens.size()){
            return std::make_unique<Token>(m_tokens[m_index]);
        }
        return nullptr;
    }
    std::unique_ptr<AST> LogError(char *error){
        fprintf(stderr, "Error: %s\n", error);
        return nullptr;
    }
    std::unique_ptr<IntAST> parseInt(){
        auto result = std::make_unique<IntAST>(fetchToken()->value.value());
        m_index++;
        return result;
    }

    std::unique_ptr<AST> parseBrac(){
        m_index++;
        auto content = parseNode();
        if(!content){ //Check for empty brackets case
            return nullptr;
        }
        if(fetchToken() && fetchToken()->type != TokenTypes::Token_CloseR){
            return LogError("')' expected");
        }
        m_index++;
        return content;
    }

    std::unique_ptr<AST> parseIdent(){ //TODO: add function parsing into here
        std::unique_ptr<Token> token = fetchToken();
        m_index++;
        if(fetchToken()->type != TokenTypes::Token_OpenR){
            return std::make_unique<IdentAST>(token->value.value());
        }
        m_index++;
        std::vector<std::unique_ptr<AST>> args;
        while(fetchToken()->type != TokenTypes::Token_CloseR){
            if(auto arg = parseNode()){
                args.push_back(std::move(parseParam()));

            }
            else{
                return nullptr;
            }
            if(fetchToken()->type != TokenTypes::Token_Comma || fetchToken()->type != TokenTypes::Token_CloseR){
                return LogError("Expected ')' or ','");
            }
            m_index++;
        }
        m_index++;
        return std::make_unique<CallAST>(token->value.value(), std::move(args));
    }

    std::unique_ptr<AST> parseParam(){ //TODO: support default value for params
        switch(fetchToken()->type){
            case TokenTypes::Token_Int_Dec: {
                m_index++;
                if (!fetchToken() || fetchToken()->type != TokenTypes::Token_Ident) {
                    return LogError("Expected a variable name after 'int'");
                }
                IdentAST ident = IdentAST(fetchToken()->value.value());
                m_index++;
                if (fetchToken() && (fetchToken()->type == TokenTypes::Token_Comma || fetchToken()->type == TokenTypes::Token_CloseR)) {
                    return std::make_unique<IntDecAST>(ident, nullptr);
                }
            }
            default:
                ;
        }
        return nullptr;
    }
    std::unique_ptr<AST> parseNode(){
        switch(fetchToken()->type){
            case TokenTypes::Token_Ident:
                return parseIdent();
            case TokenTypes::Token_Int:
                return parseInt();
            case TokenTypes::Token_OpenR:
                return parseBrac();
            default:
                return LogError("Unknown token");
        }
    }

    int getPrecendence(){
        int prec = m_precedence[fetchToken()->type];
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
            std::unique_ptr<Token> op = fetchToken();
            m_index++;
            auto RHS = parseNode();
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
            LHS = std::make_unique<BinExprAST>(std::move(op), std::move(LHS), std::move(RHS));
        }
    }
public:
    std::vector<std::unique_ptr<AST>> parseProgram(){
        std::vector<std::unique_ptr<AST>> asts;
        while(fetchToken()){
            switch(fetchToken()->type){
                case TokenTypes::Token_EOF: {
                    return asts;
                }
                case TokenTypes::Token_Semi: {
                    m_index++;
                }
                case TokenTypes::Token_Int_Dec: {
                    asts.push_back(parseIntDec());
                }
                default: {
                    //TODO
                }
            }
        }
    }

    explicit Parser(std::vector<Token> tokens){
        m_tokens = std::move(tokens);
    }

    std::unique_ptr<AST> parseBinExpr() { //TODO: DONE
        auto LHS = parseNode();
        if (!LHS) {
            return nullptr;
        }
        return parseBinRHS(0, std::move(LHS));
    }

    std::unique_ptr<AST> parseIntDec(){ //TODO: DONE
        m_index++;
        if(!fetchToken() || fetchToken()->type != TokenTypes::Token_Ident){
            return LogError("Expected a variable name after 'int'");
        }
        std::string name = fetchToken()->value.value();
        m_index++;

        //Handle assignment possibility
        if(fetchToken()->type == TokenTypes::Token_Equal){
            m_index++;
            return std::make_unique<IntDecAST>(IdentAST(name), parseBinExpr());
        }

        //Handle function declaration/definition possibility
        if(fetchToken()->type == TokenTypes::Token_OpenR){
            m_index++;
            std::vector<std::unique_ptr<AST>> params;
            while(std::unique_ptr<AST> param = parseParam()){
                params.push_back(std::move(param));
                m_index++;
            }
            std::unique_ptr<FunDecAST> dec = std::make_unique<FunDecAST>("int", name, std::move(params)); //Parse function declaration
            if(fetchToken()->type == TokenTypes::Token_OpenS){ //Parse function definition
                return std::make_unique<FunDefAST>(std::move(dec), parseProgram());
            }
            if(fetchToken()->type != TokenTypes::Token_Semi){ //Handle error
                return LogError("Expected ';'");
            }
            return dec;
        }
        return nullptr;
    }
};
