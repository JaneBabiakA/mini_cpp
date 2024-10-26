#pragma once
#include<string>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <variant>
#include "lexer.h"
#include "trees.fwd.h"
#include "trees.h"

class Parser{
    int m_index = 0;
    static inline std::map<TokenTypes, int> m_precedence = {{TokenTypes::Token_Plus, 1}, {TokenTypes::Token_Multiply, 2}};
    std::vector<Token> m_tokens;

    std::unique_ptr<Token> fetchToken();
    std::unique_ptr<ExpressionAST> LogError(char *error);
    std::unique_ptr<IntAST> parseInt();
    std::unique_ptr<ExpressionAST> parseBrac();
    std::unique_ptr<ExpressionAST> parseIdent();
    std::unique_ptr<IntDecAST> parseParam();
    std::unique_ptr<ExpressionAST> parseNode();
    int getPrecendence();
    std::unique_ptr<ExpressionAST> parseBinRHS(int prevPrec, std::unique_ptr<ExpressionAST> LHS);
    std::unique_ptr<ExpressionAST> parseBinExpr();
    std::unique_ptr<ExpressionAST> parseLclIntDec();
    std::variant<std::unique_ptr<ExpressionAST>, std::unique_ptr<FunctionAST>> parseGlblIntDec();
    std::unique_ptr<ExpressionAST> parseReturn();
    std::vector<std::unique_ptr<ExpressionAST>> parseBody();
public:
    explicit Parser(std::vector<Token> tokens){
        m_tokens = std::move(tokens);
    }

    std::vector<std::variant<std::unique_ptr<ExpressionAST>, std::unique_ptr<FunctionAST>>> parseProgram();

    std::variant<std::unique_ptr<ExpressionAST>, std::unique_ptr<FunctionAST>> LogErrorV(char *error);
};
