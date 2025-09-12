#include<optional>
#include<string>
#include<vector>
#include<iostream>
#include "lexer.h"

explicit Lexer::Lexer(std::string input){
    std::cout << input << std::endl;
    m_input = std::move(input);
}
    
std::vector<Token> Lexer::lex(){
    std::vector<Token> tokens;
    while(fetchChar().has_value()){
        if(isalpha(fetchChar().value())){ // Keywords and variable names
            std::string current = {fetchChar().value()};
            m_index++;
            while(fetchChar().has_value() && isalnum(fetchChar().value())){
                current.push_back(fetchChar().value());
                m_index++;
            }
            m_index--;
            if(current == "int"){
                tokens.push_back({TokenTypes::Token_Int_Dec});
            }
            else if(current == "return"){
                tokens.push_back({TokenTypes::Token_Return});
            }
            else if(current == "if"){
                tokens.push_back({TokenTypes::Token_If});
            }
            else if(current == "else"){
                tokens.push_back({TokenTypes::Token_Else});
            }
            else{
                tokens.push_back({TokenTypes::Token_Ident, current});
            }
        }
        else if(isdigit(fetchChar().value())){ // Integers
            std::string current = {fetchChar().value()};
            m_index++;
            while(fetchChar().has_value() && isdigit(fetchChar().value())){
                current.push_back(fetchChar().value());
                m_index++;
            }
            m_index--;
            tokens.push_back({TokenTypes::Token_Int, current});
        }
        else if(fetchChar().value() == '+'){
            tokens.push_back({TokenTypes::Token_Plus});
        }            
        else if(fetchChar().value() == '-'){
            tokens.push_back({TokenTypes::Token_Minus});
        }
        else if(fetchChar().value() == '*'){
            tokens.push_back({TokenTypes::Token_Multiply});
        }
        else if(fetchChar().value() == '/'){
            tokens.push_back({TokenTypes::Token_Divide});
        }
        else if(fetchChar().value() == '{'){
            tokens.push_back({TokenTypes::Token_OpenS});
        }
        else if(fetchChar().value() == '}'){
            tokens.push_back({TokenTypes::Token_CloseS});
        }
        else if(fetchChar().value() == ';'){
            tokens.push_back({TokenTypes::Token_Semi});
        }
        else if(fetchChar().value() == '='){
            tokens.push_back({TokenTypes::Token_Equal});
        }
        else if(fetchChar().value() == '('){
            tokens.push_back({TokenTypes::Token_OpenR});
        }
        else if(fetchChar().value() == ')'){
            tokens.push_back({TokenTypes::Token_CloseR});
        }
        else if(fetchChar().value() == ','){
            tokens.push_back({TokenTypes::Token_Comma});
        }
        m_index++;
    }
    tokens.push_back({TokenTypes::Token_EOF});
    return tokens;
}

std::optional<char> Lexer::fetchChar(){
    if(m_index < m_input.length()){
        if(m_input[m_index] == '/' && m_input[m_index + 1] == '/'){
            while(m_index < m_input.length() && m_input[m_index] != '\n'){
                m_index++; // Skip comments
            }
        }
        else{
            return m_input[m_index];
        }
    }
    return {};
}