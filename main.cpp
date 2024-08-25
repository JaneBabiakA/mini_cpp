#include<iostream>
#include<fstream>
#include<sstream>
#include"lexer.h"


int main(int argc, char* argv[]){
    if(argc != 2){
        std::cerr << "Error" << std::endl;
        return 1;
    }
    std::fstream file(argv[1]);
    std::stringstream content;
    content << file.rdbuf();
    Lexer myLexer = Lexer(content.str());
    myLexer.lex();
    return 0;
}
