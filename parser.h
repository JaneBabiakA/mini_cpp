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

class IntDecAST: public AST{ //TODO: add in scoping?
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
};

class FunDefAST: public AST{
    std::unique_ptr<FunDecAST> m_dec;
    std::vector<std::unique_ptr<AST>> m_body;
public:
    FunDefAST(std::unique_ptr<FunDecAST> dec, std::vector<std::unique_ptr<AST>> body) : m_dec(std::move(dec)), m_body(std::move(body)) {}
};

class AssignAST: public AST{
    IdentAST m_ident;
    std::unique_ptr<AST> m_value;
public:
    AssignAST(IdentAST ident, std::unique_ptr<AST> value): m_ident(std::move(ident)), m_value(std::move(value)){};
};

class ReturnAST: public AST{
    std::unique_ptr<AST> m_value;
public:
    ReturnAST(std::unique_ptr<AST> value): m_value(std::move(value)){};
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

    std::unique_ptr<AST> parseIdent(){
        std::unique_ptr<Token> token = fetchToken();
        m_index++;
        switch(fetchToken()->type){
            case TokenTypes::Token_OpenR:{
                std::vector<std::unique_ptr<AST>> args;
                while(fetchToken()->type != TokenTypes::Token_CloseR){
                    m_index++;
                    if(auto arg = parseNode()){
                        args.push_back(std::move(arg));
                    }
                    else{
                        return nullptr;
                    }
                    if(fetchToken()->type != TokenTypes::Token_Comma && fetchToken()->type != TokenTypes::Token_CloseR){
                        return LogError("Expected ')' or ','");
                    }
                }
                m_index++;
                return std::make_unique<CallAST>(token->value.value(), std::move(args));
            }
            case TokenTypes::Token_Equal: {
                m_index++;
                return std::make_unique<AssignAST>(IdentAST(token->value.value()), parseBinExpr());
            }
            default: {
                return std::make_unique<IdentAST>(token->value.value());
            }
        }
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
            case TokenTypes::Token_CloseR: {
                m_index++;
                return nullptr;
            }
            default: {
                return nullptr;
            }
        }
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
    std::unique_ptr<AST> parseBinExpr() { //TODO: DONE
        auto LHS = parseNode();
        if (!LHS) {
            return nullptr;
        }
        return parseBinRHS(0, std::move(LHS));
    }
    std::unique_ptr<AST> parseLclIntDec(){
        m_index++;
        if(!fetchToken() || fetchToken()->type != TokenTypes::Token_Ident){
            return LogError("Expected a variable name after 'int'");
        }
        std::string name = fetchToken()->value.value();
        m_index++;

        //Handle declaration possibility
        if(fetchToken()->type == TokenTypes::Token_Semi){
            m_index++;
            return std::make_unique<IntDecAST>(IdentAST(name), nullptr);
        }

        //Handle assignment possibility
        if(fetchToken()->type == TokenTypes::Token_Equal){
            m_index++;
            return std::make_unique<IntDecAST>(IdentAST(name), parseBinExpr());
        }
        return nullptr; //TODO: Better error handling
    }
    std::unique_ptr<AST> parseGlblIntDec(){
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
                m_index++;
                return std::make_unique<FunDefAST>(std::move(dec), parseBody());
            }
            if(fetchToken()->type != TokenTypes::Token_Semi){ //Handle error
                return LogError("Expected ';'");
            }
            return dec;
        }
        return nullptr;
    }
    std::unique_ptr<AST> parseReturn(){ //Verify that it is followed be a ;?
        m_index++;
        return std::make_unique<ReturnAST>(parseNode());
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
                    break;
                }
                case TokenTypes::Token_Int_Dec: {
                    asts.push_back(parseGlblIntDec());
                    break;
                }
                default: {
                    break;
                }
            }
        }
        return asts;
    }


    std::vector<std::unique_ptr<AST>> parseBody(){ //TODO: add in assignation (peanut = 2 + 3;) -> can be added into parseIdent()
        std::vector<std::unique_ptr<AST>> asts;
        while(fetchToken()){
            switch(fetchToken()->type){
                case TokenTypes::Token_Ident: {
                    asts.push_back(parseIdent());
                    break;
                }
                case TokenTypes::Token_Int_Dec: {
                    asts.push_back(parseLclIntDec());
                    break;
                }
                case TokenTypes::Token_Semi: {
                    m_index++;
                    break;
                }
                case TokenTypes::Token_CloseS: {
                    m_index++; //Close up function
                    return asts;
                }
                case TokenTypes::Token_Return: {
                    asts.push_back(parseReturn());
                }
                default: {
                    break;
                }
            }
        }
        return asts;
    }

    explicit Parser(std::vector<Token> tokens){
        m_tokens = std::move(tokens);
    }
};
