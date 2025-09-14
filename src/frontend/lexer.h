#pragma once
#include<optional>
#include<string>
#include<vector>
#include<iostream>

enum class TokenTypes {
    Token_Int,
    Token_EOF,
    Token_Ident,
    Token_Int_Dec,
    Token_Plus,
    Token_Minus,
    Token_Multiply,
    Token_Divide,
    Token_Equal,
    Token_NotEqual,
    Token_LogicalAnd,
    Token_LogicalOr,
    Token_Less,
    Token_Greater,
    Token_LessEqual,
    Token_GreaterEqual,
    Token_OpenR, //Round bracket
    Token_CloseR, //Round bracket,
    Token_OpenS, //Squiggle bracket
    Token_CloseS, //Squiggle bracket,
    Token_Semi,
    Token_Assign,
    Token_Comma,
    Token_Return,
    Token_If,
    Token_Else
};

struct Token {
    TokenTypes type;
    std::optional<std::string> value{};
};

class Lexer {
public:
    explicit Lexer(std::string input);
    std::vector<Token> lex();

private:
    std::string m_input;
    int m_index = 0;
    std::optional<char> fetchChar();
};
