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
    Token_Int_Dec,
    Token_Plus,
    Token_Multiply,
    Token_OpenR, //Round bracket
    Token_CloseR, //Round bracket,
    Token_OpenS, //Squiggle bracket
    Token_CloseS, //Squiggle bracket,
    Token_Semi,
    Token_Equal,

};

//My to do list:
//TODO: set up a gitignore file
//Add in '!' when you add booleans

struct Token{
    TokenTypes type;
    std::optional<std::string> value{};
};

class Lexer{

public:
    explicit Lexer(std::string input){
        m_input = std::move(input);
    }
    std::vector<Token> lex(){
        std::vector<Token> tokens;
        while(fetchChar().has_value()){

            //Keywords and variable names
            if(isalpha(fetchChar().value())){
                std::string current = {fetchChar().value()};
                m_index++;
                while(fetchChar().has_value() && isalnum(fetchChar().value())){
                    current.push_back(fetchChar().value());
                    m_index++;
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

            //Integers
            else if(isdigit(fetchChar().value())){ //TODO: get negative signs/numbers and decimals working
                std::string current = {fetchChar().value()};
                m_index++;
                while(fetchChar().has_value() && isdigit(fetchChar().value())){
                    current.push_back(fetchChar().value());
                    m_index++;
                }
                tokens.push_back({TokenTypes::Token_Int, current});
            }

            //Single character tokens
            else if(fetchChar().value() == '+'){
                tokens.push_back({TokenTypes::Token_Plus});
                std::cout << "+";
            }
            else if(fetchChar().value() == '*'){
                tokens.push_back({TokenTypes::Token_Multiply});
                std::cout << "*";
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
            m_index++;
        }
        return tokens;
    }

private:
    std::string m_input;
    int m_index = 0;
    std::optional<char> fetchChar(){
        if(m_index < m_input.length()){
            if(m_input[m_index] == '/' && m_input[m_index + 1] == '/'){
                while(m_index < m_input.length() && m_input[m_index] != '\n'){ // TODO: i might not need the first case?
                    m_index++; //Skip comments
                }
            }
            else{
                return m_input[m_index];
            }
        }
        return {};
    }
};
