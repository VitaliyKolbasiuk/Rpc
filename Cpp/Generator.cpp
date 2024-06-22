#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>

struct RpcArg {
    std::string type;
    bool        isVector = false;
    std::string name;
};

struct RPCFunction {
    std::vector<RpcArg> returnTypes;
    std::string         name;
    std::vector<RpcArg> argTypes;
};

RpcArg parseArg(const std::string& arg, std::string& outError)
{
    RpcArg rpcArg;

    std::regex rpcRegex(R"(\s*([\w\<\>]+)\s*(\w+)?)");
    std::smatch match;
    std::string::const_iterator searchArg(arg.cbegin());

    std::regex_search(searchArg, arg.cend(), match, rpcRegex);
    {
        rpcArg.type = match[1];
        rpcArg.name = match[2];

    }
    return rpcArg;
}

std::vector<RpcArg> parseRpcArg( const std::string& expr, std::string& outError )
{
    std::vector<RpcArg> rpcArgList;

    const char* ptr = expr.c_str();

    while (*ptr != 0)
    {
        const char* position = strchr(ptr, ',');
        if (position == nullptr)
        {
            if (strspn(ptr, " \t\r\n") == strlen(ptr))
            {
                break;
            }
            rpcArgList.emplace_back(parseArg(ptr, outError));
            break;
        }
        else
        {
            std::string arg(ptr, position - ptr);

            rpcArgList.emplace_back(parseArg(arg, outError));
            if (!outError.empty())
            {
                break;
            }
        }
        ptr = position + 1;
    }
    return rpcArgList;
}

void rpcFuncDump(RPCFunction& rpcFunction)
{
    std::cout << "(";
    for(size_t i = 0; i < rpcFunction.returnTypes.size(); ++i)
    {
        if (i == rpcFunction.returnTypes.size() - 1)
        {
            std::cout << rpcFunction.returnTypes[i].type << ' ' << rpcFunction.returnTypes[i].name;
        }
        else
        {
            std::cout << rpcFunction.returnTypes[i].type << ' ' << rpcFunction.returnTypes[i].name << ", ";
        }
    }
    std::cout << ')' << ' ' << rpcFunction.name << '(';

    for(size_t i = 0; i < rpcFunction.argTypes.size(); ++i)
    {
        if (i == rpcFunction.argTypes.size() - 1)
        {
            std::cout << rpcFunction.argTypes[i].type << ' ' << rpcFunction.argTypes[i].name;
        }
        else
        {
            std::cout << rpcFunction.argTypes[i].type << ' ' << rpcFunction.argTypes[i].name << ", ";
        }
    }
    std::cout << ')' << std::endl;
}

std::vector<RPCFunction> parseRPCDefinitions(const std::string& input) {
    std::vector<RPCFunction> rpcFunctions;
    std::regex rpcRegex(R"(rpc\s*\(([^\)]*)\)\s*(\w+)\s*\(([^\)]*)\))");
    std::smatch match;
    std::string::const_iterator searchStart(input.cbegin());

    while (std::regex_search(searchStart, input.cend(), match, rpcRegex))
    {
        RPCFunction func;

        std::string outError;
        std::vector<RpcArg> rpcReturnArgList = parseRpcArg(match[1], outError);
        func.returnTypes = rpcReturnArgList;
        func.name = match[2];

        std::vector<RpcArg> rpcArgList = parseRpcArg(match[3], outError);
        func.argTypes = rpcArgList;

        rpcFuncDump(func);

        rpcFunctions.push_back(func);
        searchStart = match.suffix().first;
    }

    return rpcFunctions;
}

void writeEasyRpcSessionBaseHeaders(std::ofstream& file)
{
    file << R"(#include "Server/EasyRpcTcpServer.h"
#include "Server/ServerSession.h"
#include "EasyRpcFunction.h"

#include <cstdint>
#include <string>
#include <memory>)";
}

void createEasyRpcSessionBase(const std::vector<RPCFunction>& rpcFunctions)
{
    for(const auto& rpcFunction : rpcFunctions)
    {
        std::ofstream file("EasyRpcSessionBase.h", std::ios::out);
        if (file.is_open())
        {

        }
    }
}

int main() {
    std::string input = R"(
        rpc (double result, string) plus( double arg1, double arg2 )
        rpc (double result) minus( double arg1, double arg2 )
        rpc (double result) plus( vector<double> args )
    )";

    std::vector<RPCFunction> rpcFunctions = parseRPCDefinitions(input);

    return 0;
}
