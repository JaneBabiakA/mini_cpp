#include "frontend/request.h"
#include<iostream>
#include<fstream>
#include<sstream>
#include <memory>
#include "llvm/Support/raw_ostream.h"
#include "frontend/lexer.h"
#include "trees.fwd.h"
#include "trees.h"
#include "frontend/parser.h"
#include "backend/codegen.h"

int main(int argc, char* argv[]){
    Request request(argc, argv);

    std::fstream file(argv[1]);
    std::stringstream content;
    content << file.rdbuf();

    Lexer myLexer = Lexer(content.str());
    std::vector<Token> tokens = myLexer.lex();

    Parser myParser = Parser(tokens);
    auto asts = myParser.parseProgram();

    GeneratorVisitor* myGenerator = new GeneratorVisitor();
    for(auto& ast : asts){
        std::visit([&](auto&& arg) {
            arg->accept(myGenerator);
        }, ast);
    }

    myGenerator->finish();
    return request.returnOutput();
}
