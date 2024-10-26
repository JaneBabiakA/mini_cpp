#include "parser.h"
#include <variant>

std::unique_ptr<Token> Parser::fetchToken(){ //TODO: figure out how to deal with end of file/end of stream
    if(m_index < m_tokens.size()){
        return std::make_unique<Token>(m_tokens[m_index]);
    }
    return nullptr;
}

std::unique_ptr<ExpressionAST> Parser::LogError(char *error){
    fprintf(stderr, "Error: %s\n", error);
    return nullptr;
}

std::variant<std::unique_ptr<ExpressionAST>, std::unique_ptr<FunctionAST>> Parser::LogErrorV(char *error) {
    throw ("Error: %s\n", error);
}

std::unique_ptr<IntAST> Parser::parseInt(){
    auto result = std::make_unique<IntAST>(fetchToken()->value.value());
    m_index++;
    return result;
}

std::unique_ptr<ExpressionAST> Parser::parseBrac(){
    m_index++;
    auto content = parseBinExpr();
    if(!content){ //Check for empty brackets case
        return nullptr;
    }
    if(fetchToken() && fetchToken()->type != TokenTypes::Token_CloseR){
        return LogError("')' expected");
    }
    m_index++;
    return content;
}

std::unique_ptr<ExpressionAST> Parser::parseIdent(){
    std::unique_ptr<Token> token = fetchToken();
    m_index++;
    switch(fetchToken()->type){
        case TokenTypes::Token_OpenR:{
            std::vector<std::unique_ptr<ExpressionAST>> args;
            while(fetchToken()->type != TokenTypes::Token_CloseR){
                m_index++;
                if(auto arg = parseNode()){
                    args.push_back(std::move(arg));
                }
                else{
                    return nullptr;
                }
                if(fetchToken()->type != TokenTypes::Token_Comma && fetchToken()->type != TokenTypes::Token_CloseR){
                    return LogError("Expected ')' or ','");
                }
            }
            m_index++;
            return std::make_unique<CallAST>(token->value.value(), std::move(args));
        }
        case TokenTypes::Token_Equal: {
            m_index++;
            return std::make_unique<AssignAST>(IdentAST(token->value.value()), parseBinExpr());
        }
        default: {
            return std::make_unique<IdentAST>(token->value.value());
        }
    }
}

std::unique_ptr<IntDecAST> Parser::parseParam(){ //TODO: support default value for params
    switch(fetchToken()->type){
        case TokenTypes::Token_Int_Dec: {
            m_index++;
            if (!fetchToken() || fetchToken()->type != TokenTypes::Token_Ident) {
                return nullptr;
            }
            IdentAST ident = IdentAST(fetchToken()->value.value());
            m_index++;
            if (fetchToken() && (fetchToken()->type == TokenTypes::Token_Comma || fetchToken()->type == TokenTypes::Token_CloseR)) {
                return std::make_unique<IntDecAST>(ident, nullptr);
            }
        }
        case TokenTypes::Token_CloseR: {
            m_index++;
            return nullptr;
        }
        default: {
            return nullptr;
        }
    }
}

std::unique_ptr<ExpressionAST> Parser::parseNode(){
    switch(fetchToken()->type){
        case TokenTypes::Token_Ident:
            return parseIdent();
        case TokenTypes::Token_Int:
            return parseInt();
        case TokenTypes::Token_OpenR:
            return parseBrac();
        default:
            return LogError("Unknown token");
    }
}

int Parser::getPrecendence(){
    int prec = m_precedence[fetchToken()->type];
    if(prec == 0){
        return -1;
    }
    return prec;
}

std::unique_ptr<ExpressionAST> Parser::parseBinRHS(int prevPrec, std::unique_ptr<ExpressionAST> LHS){
    while(true){
        int currPrec = getPrecendence();
        if(currPrec < prevPrec){
            return LHS;
        }
        std::unique_ptr<Token> op = fetchToken();
        m_index++;
        auto RHS = parseNode();
        if(!RHS){
            return nullptr;
        }
        int nextPrec = getPrecendence();
        if(currPrec < nextPrec){
            RHS = parseBinRHS(currPrec+1, std::move(RHS));
            if(!RHS){
                return nullptr;
            }
        }
        LHS = std::make_unique<BinExprAST>(std::move(op), std::move(LHS), std::move(RHS));
    }
}

std::unique_ptr<ExpressionAST> Parser::parseBinExpr() { //TODO: DONE
    auto LHS = parseNode();
    if (!LHS) {
        return nullptr;
    }
    return parseBinRHS(0, std::move(LHS));
}

std::unique_ptr<ExpressionAST> Parser::parseLclIntDec(){
    m_index++;
    if(!fetchToken() || fetchToken()->type != TokenTypes::Token_Ident){
        return LogError("Expected a variable name after 'int'");
    }
    std::string name = fetchToken()->value.value();
    m_index++;

    //Handle declaration possibility
    if(fetchToken()->type == TokenTypes::Token_Semi){
        m_index++;
        return std::make_unique<IntDecAST>(IdentAST(name), nullptr);
    }

    //Handle assignment possibility
    if(fetchToken()->type == TokenTypes::Token_Equal){
        m_index++;
        return std::make_unique<IntDecAST>(IdentAST(name), parseBinExpr());
    }
    return nullptr; //TODO: Better error handling
}

std::variant<std::unique_ptr<ExpressionAST>, std::unique_ptr<FunctionAST>> Parser::parseGlblIntDec(){
    m_index++;
    if(!fetchToken() || fetchToken()->type != TokenTypes::Token_Ident){
        return LogErrorV("Expected a variable name after 'int'");
    }
    std::string name = fetchToken()->value.value();
    m_index++;

    //Handle assignment possibility
    if(fetchToken()->type == TokenTypes::Token_Equal){
        m_index++;
        return std::make_unique<ExpressionAST>(IntDecAST(IdentAST(name), parseBinExpr()));
    }

    //Handle function declaration/definition possibility
    if(fetchToken()->type == TokenTypes::Token_OpenR){
        m_index++;
        std::vector<std::unique_ptr<IntDecAST>> params;
        while(std::unique_ptr<IntDecAST> param = parseParam()){
            params.push_back(std::move(param));
            m_index++;
        }
        std::unique_ptr<FunDecAST> dec = std::make_unique<FunDecAST>("int", name, std::move(params)); //Parse function declaration
        if(fetchToken()->type == TokenTypes::Token_OpenS){ //Parse function definition
            m_index++;
            return std::make_unique<FunDefAST>(FunDefAST(std::move(dec), parseBody()));;
        }
        if(fetchToken()->type != TokenTypes::Token_Semi){ //Handle error
            return LogErrorV("Expected ';'");
        }
        return dec;
    }
    return LogErrorV("Error of some kind");
}

std::unique_ptr<ExpressionAST> Parser::parseReturn(){ //Verify that it is followed be a ;?
    m_index++;
    return std::make_unique<ReturnAST>(parseBinExpr());
}

std::vector<std::variant<std::unique_ptr<ExpressionAST>, std::unique_ptr<FunctionAST>>> Parser::parseProgram(){
    std::vector<std::variant<std::unique_ptr<ExpressionAST>, std::unique_ptr<FunctionAST>>> asts;
    while(fetchToken()){
        switch(fetchToken()->type){
            case TokenTypes::Token_EOF: {
                return asts;
            }
            case TokenTypes::Token_Semi: {
                m_index++;
                break;
            }
            case TokenTypes::Token_Int_Dec: {
                asts.push_back(parseGlblIntDec());
                break;
            }
            default: {
                break;
            }
        }
    }
    return asts;
}

std::vector<std::unique_ptr<ExpressionAST>> Parser::parseBody(){ //TODO: add in assignation (peanut = 2 + 3;) -> can be added into parseIdent()
    std::vector<std::unique_ptr<ExpressionAST>> asts;
    while(fetchToken()){
        switch(fetchToken()->type){
            case TokenTypes::Token_Ident: {
                asts.push_back(parseIdent());
                break;
            }
            case TokenTypes::Token_Int_Dec: {
                asts.push_back(parseLclIntDec());
                break;
            }
            case TokenTypes::Token_Semi: {
                m_index++;
                break;
            }
            case TokenTypes::Token_CloseS: {
                m_index++; //Close up function
                return asts;
            }
            case TokenTypes::Token_Return: {
                asts.push_back(parseReturn());
            }
            default: {
                break;
            }
        }
    }
    return asts;
}
