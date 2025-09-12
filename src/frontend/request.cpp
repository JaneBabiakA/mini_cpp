#include "request.h"
#include <stdexcept>
#include <sys/stat.h>
#include <iostream>

Request::Request(int length, char* command[]){
    if(length < 2){
        throw std::invalid_argument("Input file required");
    }
    struct stat buffer;
    std::cout << command[1] << std::endl;
    if(stat(command[1], &buffer)){
        throw std::invalid_argument(std::string("Cannot find input file ") + command[1]);
    }

    // Insert all possible compiler flags into arguments map
    args["-o"];

    int argPos = 2;
    while(argPos < length - 1){
        if(!args.contains(command[argPos])){
            throw std::invalid_argument("Compiler flag '");
            // throw std::invalid_argument("Compiler flag '" + *command[argPos]);
        }
        else if(args.contains(command[argPos + 1])){
            throw std::invalid_argument("Missing value for compiler flag '");
            // throw std::invalid_argument("Missing value for compiler flag '" + *command[argPos]);
        }
        else {
            args[command[argPos]] = command[argPos + 1];
            ++argPos;
        }
        ++argPos;
    }
}

int Request::returnOutput(){
    std::string file = "b.out";
    if(args["-o"] != ""){
        file = args["-o"];
    }
    std::cout << std::system(("clang output.o -o " + file).c_str());
    return 0;
}