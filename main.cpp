#include<iostream>
#include<fstream>
#include<sstream>
#include <memory>
#include "llvm/Support/raw_ostream.h"
#include"lexer.h"
#include "trees.fwd.h"
#include "trees.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char* argv[]){
    if(argc != 2){
        std::cerr << "Error" << std::endl;
        return 1;
    }
    std::fstream file(argv[1]);

    //Set up for testing binary expressions
    std::stringstream content;
    content << file.rdbuf();
    Lexer myLexer = Lexer(content.str());
    std::vector<Token> tokens = myLexer.lex();
    Parser myParser = Parser(tokens);
    auto result = myParser.parseProgram();
    GeneratorVisitor* myGenerator = new GeneratorVisitor();


    auto mainFunction = std::move(std::get<std::unique_ptr<FunctionAST>>(result[0]));
    auto testFunction = std::move(std::get<std::unique_ptr<FunctionAST>>(result[1]));
    auto* test = mainFunction->accept(myGenerator);
    auto* test1 = testFunction->accept(myGenerator);
    test->print(llvm::errs());
    test1->print(llvm::errs());
    return 0;
}
