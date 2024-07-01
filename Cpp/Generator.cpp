#include <complex>
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
        int nameIndex = 2;
        for (auto& f : rpcFunctions)
        {
            if (f.name == func.name)
            {
                func.name = match[2].str() + std::to_string(nameIndex);
                nameIndex++;
            }
        }

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
    file << "\n\n";
}

void startGeneratingSessionBase(std::ofstream& file)
{
    file << R"(class EasyRpcSessionBase : public ServerSession
{
public:
    virtual ~EasyRpcSessionBase() = default;
)";
    file << '\n';
}

void generateVirtualFunctions(std::ostream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    for (const auto& rpcFunction : rpcFunctions)
    {
        file << "\tvirtual void on_" << rpcFunction.name << "(uint64_t context, ";
        for (size_t i = 0; i < rpcFunction.argTypes.size(); ++i)
        {
            // TODO if vector
            file << rpcFunction.argTypes[i].type << ' ' << rpcFunction.argTypes[i].name;
            if (i == rpcFunction.argTypes.size() - 1)
            {
                file << ')';
            }
            else
            {
                file << ", ";
            }
        }
        file << ";\n";
    }
}

void startGenerateOnPacketReceived(std::ostream& file)
{
    file << R"(// Virtual
    void onPacketReceived(std::shared_ptr<std::string> packet) override
    {
        // Detect operation/call
        auto* ptr = const_cast<char*>(packet->c_str());
        const char* packetEnd = ptr + packet->size();

        uint16_t operation;
        readFromPacket(operation, &ptr, packetEnd);

        uint64_t context;
        readFromPacket(context, &ptr, packetEnd);)";
    file << '\n';
}

void generateSwitchServer(std::ostream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    file << "\t\t";
    file << R"(switch(operation)
        {)";
    file << '\n';

    for(const auto& rpcFunction : rpcFunctions)
    {
        file << "\t\t\tcase EasyRpcFunction::" << rpcFunction.name << ":\n\t\t\t{\n";
        for (const auto& argType : rpcFunction.argTypes)
        {
            file << "\t\t\t\t" << argType.type << ' ' << argType.name << ";\n";
            file << "\t\t\t\treadFromPacket(" << argType.name << ", &ptr, packetEnd);\n";
        }
        file << "\t\t\t\t" << "on_" << rpcFunction.name << "(context, ";
        for (size_t i = 0; i < rpcFunction.argTypes.size(); ++i)
        {
            file << rpcFunction.argTypes[i].name;

            if (i == rpcFunction.argTypes.size() - 1)
            {
                file << ");\n";
            }
            else
            {
                file << ", ";
            }
        }
        file << "\t\t\t\t   break;\n\t\t\t}\n";
    }
    file << "\t\t\t";
    file << R"(default:
            {
                    // Close connection
                    break;
            }
        //::generateSwitch::end::
        })";
    file << "\n\t}\n\n";
}

void generateResponseFunctions(std::ostream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    for(const auto& rpcFunction : rpcFunctions)
    {
        file << "\tvoid send_" << rpcFunction.name << "_response(const uint64_t context, ";
        for(size_t i = 0; i < rpcFunction.returnTypes.size(); ++i)
        {
            file << "const " << rpcFunction.returnTypes[i].type << ' ' << rpcFunction.returnTypes[i].name;
            if (i == rpcFunction.returnTypes.size() - 1)
            {
                file << ")\n\t{\n";
            }
            else
            {
                file << ", ";
            }
        }
        file << "\t\t";
        file << R"(if (error_message.size() > 0xffff)
        {
            throw std::runtime_error("Too long string in send_plus_response");
        }
        std::string buffer;

        buffer.resize(sizeof(context) + )";
        for (size_t i = 0; i < rpcFunction.returnTypes.size(); ++i)
        {
            if (rpcFunction.returnTypes[i].type == "string")
            {
                file << "2 + " << rpcFunction.returnTypes[i].name << ".size()";
            }
            else
            {
                file << "sizeof(" << rpcFunction.returnTypes[i].name << ")";
            }

            if (i == rpcFunction.returnTypes.size() - 1)
            {
                file << ");\n";
            }
            else
            {
                file << " + ";
            }
        }
        file << "\t\t";
        file << R"(auto* ptr = const_cast<char*>(buffer.c_str());

        uint16_t operation = EasyRpcFunction::)";
        file << rpcFunction.name << ";\n\t\t";
        file << R"(write(operation, &ptr);
        write(context, &ptr);
)";
        for (const auto& arg : rpcFunction.returnTypes)
        {
            file << "\t\twrite(" << arg.name << ", &ptr);\n";
        }
        file << R"(
        sendPacket(buffer);)";
        file << "\n}\n\n";
    }
}

void startGeneratingPrivate(std::ofstream& file)
{
    file << R"(private:
    void readFromPacket(uint16_t& value, char** ptr, const char* packetEnd)
    {
        if (packetEnd <= *ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, *ptr, sizeof(value));
        *ptr += sizeof(value);
    }

    void readFromPacket(uint64_t& value, char** ptr, const char* packetEnd)
    {
        if (packetEnd <= *ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, *ptr, sizeof(value));
        *ptr += sizeof(value);
    }

)";
}

void generateReadFromPacket(std::ostream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    std::vector<std::string> writtenTypes;
    for(const auto& rpcFunction : rpcFunctions)
    {
        for (const auto& argType : rpcFunction.argTypes)
        {
            if (std::find(writtenTypes.begin(), writtenTypes.end(), argType.type) != writtenTypes.end())
            {
                continue;
            }
            file << "\tvoid readFromPacket(" << argType.type << "& value, char** ptr, const char* packetEnd)\n";
            file << "\t{\n\t\t";
            file << R"(if (packetEnd < *ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, *ptr, sizeof(value));
        *ptr += sizeof(value);
    })";
            file << "\n\n";
            writtenTypes.emplace_back(argType.type);
        }
    }
}

void startGeneratingWrite(std::ofstream& file)
{
    file << '\t';;
    file << R"(void write(const uint16_t& value, char** ptr)
    {
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);
    }

    void write(const uint64_t& value, char** ptr)
    {
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);
    })";
    file << "\n\n";
}

void generateWrite(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    std::vector<std::string> writtenTypes;
    for (const auto& rpcFunction : rpcFunctions)
    {
        for (const auto& returnType : rpcFunction.returnTypes)
        {
            if (std::find(writtenTypes.begin(), writtenTypes.end(), returnType.type) != writtenTypes.end())
            {
                continue;
            }
            if (returnType.type == "string")
            {
                file << "\t";
                file << R"(void write(const std::string& str, char** ptr)
    {
        uint16_t length = str.size();

        std::memcpy(*ptr, &length, sizeof(length));
        *ptr += sizeof(length);

        std::memcpy(*ptr, str.c_str(), length);
        *ptr += length;
    })";
            }
            else
            {
                file << "\tvoid write(const " << returnType.type << "& value, char** ptr)\n\t";
                file << R"({
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);
    })";
            }
            file << "\n\n";
            writtenTypes.emplace_back(returnType.type);
        }
    }

    file << "};";
}

void createEasyRpcFunction(const std::vector<RPCFunction>& rpcFunctions)
{
    std::ofstream file("EasyRpcFunction.h", std::ios::out);
    file << R"(#pragma once

#include <cstdint>

enum EasyRpcFunction: uint16_t
{)";
    file << '\n';
    for(size_t i = 0; i < rpcFunctions.size(); ++i)
    {
        file << '\t';
        if(i == 0)
        {
            file << rpcFunctions[i].name << " = 0";
        }
        else
        {
            file << rpcFunctions[i].name;
        }

        if (i != rpcFunctions.size() - 1)
        {
            file << ',';
        }
        file << '\n';
    }
    file << "};";
}

void createEasyRpcSessionBase(const std::vector<RPCFunction>& rpcFunctions)
{
    std::ofstream file("EasyRpcSessionBase.h", std::ios::out);
    if (file.is_open())
    {
        writeEasyRpcSessionBaseHeaders(file);
        startGeneratingSessionBase(file);
        generateVirtualFunctions(file, rpcFunctions);
        startGenerateOnPacketReceived(file);
        generateSwitchServer(file, rpcFunctions);
        generateResponseFunctions(file, rpcFunctions);
        startGeneratingPrivate(file);
        generateReadFromPacket(file, rpcFunctions);
        startGeneratingWrite(file);
        generateWrite(file, rpcFunctions);
    }
}

void writeEasyRpcClientBaseHeaders(std::ofstream& file)
{
    file << R"(#pragma once

#include "Client/TcpClient.h"
#include "EasyRpcFunction.h"

#include <cstdint>
#include <string>
#include <memory>
#include <map>
)";
    file << '\n';
}

void generateMembers(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    for(const auto& rpcFunction : rpcFunctions)
    {
        file << "\tuint64_t m_" << rpcFunction.name << "_context = 0;\n";
        file << "\tstd::map<uint64_t, std::function<void(";
        for(size_t i = 0; i < rpcFunction.returnTypes.size(); ++i)
        {
            file << rpcFunction.returnTypes[i].type << ' ' << rpcFunction.returnTypes[i].name;
            if (i != rpcFunction.returnTypes.size() - 1)
            {
                file << ", ";
            }
        }
        file << ")>> m_" << rpcFunction.name << "_map;\n";
        file << "\tstd::mutex m_" << rpcFunction.name << "_map_mutex;\n\n";
    }

}

void startCreatingEasyRpcClientBase(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    file << R"(class EasyRpcClientBase : public TcpClient
{)";
    file << '\n';

    generateMembers(file, rpcFunctions);

    file << R"(public:
    EasyRpcClientBase() {}
    virtual ~EasyRpcClientBase() = default;
)";
}

void generateSwitchClient(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    file << "\t\tswitch(operation)\n\t\t{\n";
    for(const auto& rpcFunction : rpcFunctions)
    {
        file << "\t\t\tcaseEasyRpcFunction::" << rpcFunction.name << ":\n\t\t\t{\n";

        for(const auto& returnType : rpcFunction.returnTypes)
        {
            file << "\t\t\t\t" << returnType.type << ' ' << returnType.name << ";\n";

            if (returnType.type == "string")
            {
                file << "\t\t\t\tptrdiff_t ptrDiff = ptr - packet;\n";
                file << "\t\t\t\treadFromPacket(" << returnType.name << ", ptr, packetSize - ptrDiff);\n\n";
            }
            else
            {
                file << "\t\t\t\treadFromPacket(" << returnType.name << ", ptr, packetSize);\n\n";
            }
        }

        file << "\t\t\t\tm_" << rpcFunction.name << "_map[context](";
        for(size_t i = 0; i < rpcFunction.returnTypes.size(); ++i)
        {
            file << rpcFunction.returnTypes[i].name;
            if (i != rpcFunction.returnTypes.size() - 1)
            {
                file << ", ";
            }
        }
        file << ");\n\t\t\t\tbreak;\n";
        file << "\t\t\t}\n";
    }
    file << "\t\t}\n";
}

void generateOnPacketReceived(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    file << '\n';
    file << R"(    // Virtual
    void onPacketReceived(uint8_t* packet, const uint32_t packetSize) override
    {
        uint8_t* ptr = packet;

        uint16_t operation;
        readFromPacket(operation, ptr, packetSize);

        uint64_t context;
        readFromPacket(context, ptr, packetSize);)";
    file << '\n';

    generateSwitchClient(file, rpcFunctions);

    file << "\t\tdelete packet;\n\t}\n\n";
}

void generateFunctions(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    for(const auto& rpcFunction : rpcFunctions)
    {
        file << "\tvoid " << rpcFunction.name << "(";
        for(const auto& argType : rpcFunction.argTypes)
        {
            file << argType.type << ' ' << argType.name << ", ";
        }
        file << "std::function<void(";
        for(size_t i = 0; i < rpcFunction.returnTypes.size(); ++i)
        {
            file << rpcFunction.returnTypes[i].type << ' ' << rpcFunction.returnTypes[i].name;
            if (i != rpcFunction.returnTypes.size() - 1)
            {
                file << ", ";
            }
        }
        file << ")> func )\n\t{\n";
        file << "\t\t{\n";
        file << "\t\t\tstd::lock_guard<std::mutex> lock_guard(m_" << rpcFunction.name << "_map_mutex);\n\n";
        file << "\t\t\t++m_" << rpcFunction.name << "_context;\n";
        file << "\t\t\tm_" << rpcFunction.name << "_map[m_" << rpcFunction.name << "_context] = func;\n";
        file << "\t\t}\n\n\t\t";
        file << R"(std::string buffer;
        enum EasyRpcFunction operation = EasyRpcFunction::minus;)";
        file << "\n\n\t\tbuffer.resize(sizeof(operation) + sizeof(m_" << rpcFunction.name << "_context)";
        for (const auto& argType : rpcFunction.argTypes)
        {
            if (argType.type == "string" || argType.isVector)
            {
                file << " + 2 + " << argType.name << ".size()";
            }
            else
            {
                file << " + sizeof(" << argType.name << ')';
            }
        }
        file << ");\n\t\t";
        file << R"(auto* ptr = const_cast<char*>(buffer.c_str());
        write(operation, &ptr);)";
        file << "\n\t\twrite(m_" << rpcFunction.name << "_context, &ptr);\n";
        for (const auto& argType : rpcFunction.argTypes)
        {
            file << "\t\twrite(" << argType.name << ", &ptr);\n";
        }
        file << "\n\t\tsendPacket(buffer);\n\t}\n\n";
    }
}

void generateReadFromPacketClient(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    file << '\t';
    file << R"(void readFromPacket(uint16_t& value, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, ptr, sizeof(value));
        ptr += sizeof(value);
    }

    void readFromPacket(uint64_t& value, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, ptr, sizeof(value));
        ptr += sizeof(value);
    })";

    file << "\n\n";
    std::vector<std::string> writtenTypes;
    for (const auto& rpcFunction : rpcFunctions)
    {
        for(const auto& returnType : rpcFunction.returnTypes)
        {
            if (std::find(writtenTypes.begin(), writtenTypes.end(), returnType.type) != writtenTypes.end())
            {
                continue;
            }
            file << "\tvoid readFromPacket(" << returnType.type << "& value, uint8_t*& ptr, const uint32_t packetSize)";
            file << "\n\t{\n";
            if (returnType.type == "string" || returnType.isVector)
            {
                // TODO
            }
            else
            {
                file << R"(
        if (ptr + packetSize <= ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, ptr, sizeof(value));
        ptr += sizeof(value);)";
            }
            file << "\n\t}\n\n";
            writtenTypes.emplace_back(returnType.type);
        }
    }
}

void generateWriteClient(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    file << '\t';
    file << R"(void write(const uint16_t& value, char** ptr)
    {
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);
    }

    void write(const uint64_t& value, char** ptr)
    {
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);
    })";

    file << "\n\n";
    std::vector<std::string> writtenTypes;
    for(const auto& rpcFunction : rpcFunctions)
    {
        for(const auto& argType : rpcFunction.argTypes)
        {
            if(std::find(writtenTypes.begin(), writtenTypes.end(), argType.type) != writtenTypes.end())
            {
                continue;
            }

            file << '\t';
            if (argType.type == "string" || argType.isVector)
            {
                file << "void write(const std::" << argType.type << "& data, char** ptr)\n\t";
                file << R"({
        uint16_t length = data.size();

        std::memcpy(*ptr, &length, sizeof(length));
        *ptr += sizeof(length);

        std::memcpy(*ptr, &data[0], length);
        *ptr += length;)";
            }
            else
            {
                file << "void write(const " << argType.type << "& value, char** ptr)\n\t";
                file << R"({
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);)";
            }
            file << "\n\t}\n\n";

            writtenTypes.emplace_back(argType.type);
        }
    }
}

void generatePrivate(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    file << "private:\n\n";

    generateReadFromPacketClient(file, rpcFunctions);
    generateWriteClient(file, rpcFunctions);

    file << "};";
}

void createEasyRpcClientBase(const std::vector<RPCFunction>& rpcFunctions)
{
    std::ofstream file("EasyRpcClientBase.h", std::ios::out);
    if (file.is_open())
    {
        writeEasyRpcClientBaseHeaders(file);
        startCreatingEasyRpcClientBase(file, rpcFunctions);
        generateOnPacketReceived(file, rpcFunctions);
        generateFunctions(file, rpcFunctions);
        generatePrivate(file, rpcFunctions);
    }
}

void generateFiles(const std::vector<RPCFunction>& rpcFunctions)
{
    createEasyRpcFunction(rpcFunctions);
    createEasyRpcSessionBase(rpcFunctions);
    createEasyRpcClientBase(rpcFunctions);
}

int main() {
    std::string input = R"(
        rpc (double result, string error) plus( double arg1, double arg2 )
        rpc (double result) minus( double arg1, double arg2 )
        rpc (double result) plus( vector<double> args )
    )";

    std::vector<RPCFunction> rpcFunctions = parseRPCDefinitions(input);
    generateFiles(rpcFunctions);

    return 0;
}
