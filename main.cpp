#include<iostream>
#include<fstream>
#include<sstream>
#include"lexer.h"
#include "parser.h"


int main(int argc, char* argv[]){
    if(argc != 2){
        std::cerr << "Error" << std::endl;
        return 1;
    }
    std::fstream file(argv[1]);

    //Set up for testing binary expressions
    std::stringstream content;
    //content << file.rdbuf();
    content << "12 * 52 + 23";
    Lexer myLexer = Lexer(content.str());
    std::vector<Token> tokens = myLexer.lex();
    Parser myParser = Parser(tokens);
    std::unique_ptr<AST> result = myParser.parseBinExpr();
    return 0;
}
