#pragma once
#include <unordered_map>
#include <string>

class Request {
    std::unordered_map<std::string, std::string> args;
public:
    Request(int length, char* command[]);

    int returnOutput();
};