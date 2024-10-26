#include<iostream>
#include<fstream>
#include<sstream>
#include <memory>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include"lexer.h"
#include "trees.fwd.h"
#include "trees.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char* argv[]){
    if(argc != 2){
        //std::cerr << "Error" << std::endl;
        //return 1;
        ;
    }
    std::fstream file(argv[1]);

    //Set up for testing binary expressions
    std::stringstream content;
    //content << file.rdbuf();
    content << "int main(){}";
    Lexer myLexer = Lexer(content.str());
    std::vector<Token> tokens = myLexer.lex();
    Parser myParser = Parser(tokens);

    auto result = myParser.parseProgram();
    GeneratorVisitor* myGenerator = new GeneratorVisitor();
    auto yeah = std::move(std::get<std::unique_ptr<FunctionAST>>(result[0]));
    auto* test = yeah->accept(myGenerator);
    std::cout << "in main?" << std::endl;
    test->print(llvm::errs());
    return 0;
}

//TODO: Add in negative numbers
//TODO: Set up a gitignore file
//TODO: Add in '<' and '>'
//TODO: change headers to cpp files
//Remove all token references from trees
