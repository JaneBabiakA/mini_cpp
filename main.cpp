#include"lexer.h"

int main(){
    Lexer myLexer = Lexer("hello world number 923");
    myLexer.lex();
    return 0;
}
