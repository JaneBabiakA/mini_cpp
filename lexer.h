#pragma once
#include<optional>
#include<string>
#include<vector>
#include<iostream>
enum class TokenTypes {
    Token_Int,
    Token_EOF,
    Token_Ident,
    Token_Def,
    Token_Int_Dec
};

struct Token{
    TokenTypes type;
    std::optional<std::string> value{};
};


class Lexer{

public:
    Lexer(std::string input){
        m_input = std::move(input);
    }

    char getNext(){
        if(m_input[m_index] == '/' && m_input[m_index + 1] == '/'){ //TODO: check if we're exceeding the file length
            while(m_index < m_input.length() && m_input[m_index] != '\n'){
                m_index++;
            }
        }
        else{
            return m_input[m_index++];
        }
    }
    std::vector<Token> lex(){
        std::vector<Token> tokens;
        while(m_index < m_input.length()){
            if(isalpha(m_input[m_index])){
                std::string current = {getNext()};
                while(m_index < m_input.length() && isalnum(m_input[m_index])){
                    current.push_back(getNext());
                }
                if(current == "int"){
                    tokens.push_back({TokenTypes::Token_Int_Dec});
                }
                else if(current == "def"){
                    tokens.push_back({TokenTypes::Token_Def});
                }
                else{
                    tokens.push_back({TokenTypes::Token_Ident, current});
                }
            }
            //TODO: get negative signs/numbers and decimals working
            if(isdigit(m_input[m_index])){
                std::string current = {getNext()};
                while(m_index < m_input.length() && isdigit(m_input[m_index])){
                    current.push_back(getNext());
                }
                tokens.push_back({TokenTypes::Token_Int, current});
            }
            getNext();
        }
        return tokens;
    }

private:
    std::string m_input;
    int m_index = 0;
};
