#include <complex>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>

#include "EasyRpcFunction.h"

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

        if (rpcArg.type.substr(0, 6) == "vector")
        {
            rpcArg.isVector = true;
        }
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
    file << R"(#pragma once

#include "Server/EasyRpcTcpServer.h"
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
            if (rpcFunction.argTypes[i].type == "string" || rpcFunction.argTypes[i].isVector)
            {
                file << "std::";
            }

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
        file << " = 0;\n";
    }
}

void startGenerateOnPacketReceived(std::ostream& file)
{
    file << "\n\t";
    file << R"(// Virtual
    void onPacketReceived(std::shared_ptr<std::string> packet) override
    {
        // Detect operation/call
        auto* ptr = const_cast<char*>(packet->c_str());
        const char* packetEnd = ptr + packet->size();

        EasyRpcFunction operation;
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
            file << "\t\t\t\t";
            if (argType.type == "string" || argType.isVector)
            {
                file << "std::";
            }
            file << argType.type << ' ' << argType.name << ";\n";
            file << "\t\t\t\treadFromPacket(" << argType.name << ", &ptr, packetEnd);\n\n";
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
        file << "\t\t\t\tbreak;\n\t\t\t}\n";
    }
    file << "\t\t\t";
    file << R"(default:
            {
                closeConnection();
                break;
            }
        })";
    file << "\n\t}\n\n";
}

void generateResponseFunctions(std::ostream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    for(const auto& rpcFunction : rpcFunctions)
    {
        file << "\tvoid send_" << rpcFunction.name << "_response(const uint64_t context, ";
        std::vector<std::string> returnTypesString;
        for(size_t i = 0; i < rpcFunction.returnTypes.size(); ++i)
        {
            file << "const ";

            if (rpcFunction.returnTypes[i].type == "string" || rpcFunction.returnTypes[i].isVector)
            {
                file << "std::";
                returnTypesString.emplace_back(rpcFunction.returnTypes[i].name);
            }
            file << rpcFunction.returnTypes[i].type << ' ' << rpcFunction.returnTypes[i].name;
            if (i == rpcFunction.returnTypes.size() - 1)
            {
                file << ")\n\t{";
            }
            else
            {
                file << ", ";
            }
        }

        for(const auto& typeString : returnTypesString)
        {
            file << "\n\t\tif(" << typeString;
            file << R"(.size() > 0xffff)
        {
            throw std::runtime_error("Too long string in send_plus_response");
        })";
        }

        file << "\n\t\tstd::string buffer;";
        file << "\n\t\tbuffer.resize(sizeof(EasyRpcFunctiong) + sizeof(context) + ";
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

        EasyRpcFunction operation = EasyRpcFunction::)";
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
    void readFromPacket(EasyRpcFunction& operation, char** ptr, const char* packetEnd)
    {
        if (packetEnd <= *ptr + sizeof(operation))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&operation, *ptr, sizeof(operation));
        *ptr += sizeof(operation);
    }

    void readFromPacket(uint64_t& context, char** ptr, const char* packetEnd)
    {
        if (packetEnd <= *ptr + sizeof(context))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&context, *ptr, sizeof(context));
        *ptr += sizeof(context);
    }

)";
}

void generateReadFromPacketServer(std::ostream& file, const std::vector<RPCFunction>& rpcFunctions)
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

            if (argType.type == "string" || argType.isVector)
            {
                file << "\tvoid readFromPacket(std::" << argType.type << "& data, char** ptr, const char* packetEnd)\n";
                file << "\t{\n\t\t";
                file << "uint16_t length;\n\t\t\n\t\t";
                file << R"(if (packetEnd < *ptr + sizeof(length))
    	{
    		throw std::runtime_error("Buffer too small");
    	}
)";
                file << "\n\t\tstd::memcpy(&length, *ptr, sizeof(length));\n\t\t";
                file << "*ptr += sizeof(length);\n\t\t\n\t\t";
                file << R"(if (packetEnd < *ptr + length)
        {
            throw std::runtime_error("Buffer too small");
        }
)";
                file << "\n\t\tstd::memcpy(&data[0], *ptr, length);\n\t\t";
                file << "*ptr += length;\n\t}";
            }
            else
            {
                file << "\tvoid readFromPacket(" << argType.type << "& value, char** ptr, const char* packetEnd)\n";
                file << "\t{\n\t\t";
                file << R"(if (packetEnd < *ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, *ptr, sizeof(value));
        *ptr += sizeof(value);
    })";
            }
            file << "\n\n";
            writtenTypes.emplace_back(argType.type);
        }
    }
}

void startGeneratingWrite(std::ofstream& file)
{
    file << '\t';;
    file << R"(void write(const EasyRpcFunction& operation, char** ptr)
    {
        std::memcpy(*ptr, &operation, sizeof(operation));
        *ptr += sizeof(operation);
    }

    void write(const uint64_t& context, char** ptr)
    {
        std::memcpy(*ptr, &context, sizeof(context));
        *ptr += sizeof(context);
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
        generateReadFromPacketServer(file, rpcFunctions);
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
            if (rpcFunction.returnTypes[i].type == "string" || rpcFunction.returnTypes[i].isVector)
            {
                file << "std::";
            }
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
        file << "\t\t\tcase EasyRpcFunction::" << rpcFunction.name << ":\n\t\t\t{\n";

        for(const auto& returnType : rpcFunction.returnTypes)
        {
            file << "\t\t\t\t";
            if (returnType.type == "string" || returnType.isVector)
            {
                file << "std::";
            }
            file << returnType.type << ' ' << returnType.name << ";\n";
            file << "\t\t\t\treadFromPacket(" << returnType.name << ", ptr, packetSize);\n\n";
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

        EasyRpcFunction operation;
        readFromPacket(operation, ptr, packetSize);

        uint64_t context;
        readFromPacket(context, ptr, packetSize);)";
    file << '\n';

    generateSwitchClient(file, rpcFunctions);

    file << "\t\tdelete[] packet;\n\t}\n\n";
}

void generateFunctions(std::ofstream& file, const std::vector<RPCFunction>& rpcFunctions)
{
    for(const auto& rpcFunction : rpcFunctions)
    {
        file << "\tvoid " << rpcFunction.name << "(";
        for(const auto& argType : rpcFunction.argTypes)
        {
            if (argType.type == "string" || argType.isVector)
            {
                file << "std::";
            }
            file << argType.type << ' ' << argType.name << ", ";
        }
        file << "std::function<void(";
        for(size_t i = 0; i < rpcFunction.returnTypes.size(); ++i)
        {
            if (rpcFunction.returnTypes[i].type == "string" || rpcFunction.returnTypes[i].isVector)
            {
                file << "std::";
            }
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
        EasyRpcFunction operation = EasyRpcFunction::)";
        file << rpcFunction.name << ";";
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
    file << R"(void readFromPacket(EasyRpcFunction& operation, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(operation))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&operation, ptr, sizeof(operation));
        ptr += sizeof(operation);
    }

    void readFromPacket(uint64_t& context, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(context))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&context, ptr, sizeof(context));
        ptr += sizeof(context);
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

            if (returnType.type == "string" || returnType.isVector)
            {
                file << "\tvoid readFromPacket(std::" << returnType.type << "& data, uint8_t*& ptr, const uint32_t packetSize)";
                file << "\n\t{";
                file << R"(
        uint16_t length;

        if (ptr + packetSize <= ptr + sizeof(length))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&length, ptr, sizeof(length));
        ptr += sizeof(length);

        if (ptr + packetSize <= ptr + length)
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&data[0], ptr, length);
        ptr += length;)";
            }
            else
            {
                file << "\tvoid readFromPacket(" << returnType.type << "& value, uint8_t*& ptr, const uint32_t packetSize)";
                file << "\n\t{\n";
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

void createTcpClient()
{
    std::ofstream file("Client/TcpClient.h", std::ios::out);
    file << R"(#pragma once

#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class TcpClient
{
protected:
    io_context  m_ioContext;

private:
    tcp::socket m_socket;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_workGuard;
public:

    TcpClient() :
            m_ioContext(),
            m_socket(m_ioContext),
            m_workGuard(boost::asio::make_work_guard(m_ioContext))
    {}

    ~TcpClient()
    {
        std::cout << "!!!! ~Client(): " << std::endl;
    }

    virtual void onPacketReceived(uint8_t* packet, const uint32_t packetSize) = 0;
    virtual void onSocketConnected() {}
    virtual void onConnectionClosed() = 0;

    void connect(const std::string& addr, const int& port, std::promise<boost::system::error_code>& promise)
    {
        std::cout << "Connect: " << addr << ' ' << port << std::endl;
        auto endpoint = tcp::endpoint(ip::address::from_string( addr.c_str()), port);

        m_socket.async_connect(endpoint, [this, &promise] (const boost::system::error_code& error)
        {
            if ( error )
            {
                std::cerr <<"Connection error: " << error.message() << std::endl;
                promise.set_value(error);
            }
            else
            {
                std::cout << "Connection established" << std::endl;
                onSocketConnected();
                promise.set_value(error);
                readPacket();
            }
        });

    }

    void run()
    {
        m_ioContext.run();
    }

    std::mutex m_mutex;
    void sendPacket(const std::string& buffer)
    {
        uint32_t packetSize = buffer.size();

        std::lock_guard<std::mutex> lock_guard(m_mutex);
        boost::asio::write(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));
        boost::asio::write(m_socket, boost::asio::buffer(buffer.c_str(), packetSize));
    }

    void readPacket()
    {
        auto packetSize = std::make_shared<uint32_t>(0);
        boost::asio::async_read(m_socket, boost::asio::buffer(packetSize.get(), sizeof(*packetSize)),
                                transfer_exactly(sizeof(*packetSize)),
                                [this, packetSize](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                std::cout << "Async_read bytes transferred: " << bytes_transferred << std::endl;

                if (ec)
                {
                    std::cerr << "Read packet error: " << ec.message() << std::endl;

                    if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
                    {
                        std::cerr << "Connection closed by server" << std::endl;
                        onConnectionClosed();
                    }

                    return;
                }
                if (*packetSize == 0)
                {
                    std::cerr << "Bad packet" << std::endl;
                    return;
                }

                uint8_t* packet = new uint8_t[*packetSize];
                boost::asio::async_read(m_socket, boost::asio::buffer(packet, *packetSize), transfer_exactly(*packetSize),
                    [this, packet, packetSize](const boost::system::error_code& ec, std::size_t bytes_transferred)
                    {

                        if (ec)
                        {
                            std::cerr << "Read packet error: " << ec.message() << std::endl;

                            if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset)
                            {
                                std::cerr << "Connection closed by server" << std::endl;
                                onConnectionClosed();
                            }

                            delete[] packet;
                            return;
                        }

                        onPacketReceived(packet, *packetSize);
                        readPacket();
                    });
            });
    }

    void closeConnection()
    {
        boost::system::error_code ec;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec && ec != boost::asio::error::not_connected)
        {
            std::cerr << "Shutdown error: " << ec.message() << std::endl;
        }

        m_socket.close(ec);
        if (ec && ec != boost::asio::error::not_connected)
        {
            std::cerr << "Close error: " << ec.message() << std::endl;
        }
    }
};)";
}

void createRpcClient()
{
    std::ofstream rpcClientHeader("Client/RpcClient.h", std::ios::out);
    rpcClientHeader << R"(#pragma once

#include "EasyRpcClientBase.h"

class TcpClient;
class RpcClient : public EasyRpcClientBase
{
    std::thread m_ioContextThread;
    boost::system::error_code m_connectionError;

public:
    void start(const std::string& addr, int portNum);
    boost::system::error_code& connectionError();
    void wait();
    void stop();
    void onSocketConnected() override;
    void onConnectionClosed() override;
};)";

    std::ofstream rpcClientCpp("Client/RpcClient.cpp", std::ios::out);
    rpcClientCpp << R"(#include "RpcClient.h"
#include "TcpClient.h"

#include <iostream>

void RpcClient::start(const std::string& addr, int portNum)
{
    std::promise<boost::system::error_code> promise;

    m_ioContextThread = std::thread([this, addr, portNum, &promise](){
        connect(addr, portNum, promise);
        m_ioContext.run();
    });

    m_connectionError = promise.get_future().get();
}

void RpcClient::wait()
{
    m_ioContextThread.join();
}

void RpcClient::stop()
{
    m_ioContext.stop();
}

boost::system::error_code& RpcClient::connectionError()
{
    return m_connectionError;
}

void RpcClient::onSocketConnected()
{
    std::cout << "Socket connected" << std::endl;
}

// Virtual
void RpcClient::onConnectionClosed()
{
    std::cerr << "Connection closed";
    closeConnection();
    stop();
})";
}

void createClientFolder()
{
    if(std::filesystem::create_directory("Client"))
    {
        createTcpClient();
        createRpcClient();
    }
    else
    {
        std::cerr << "Folder \"Client\" couldn't be created";
    }
}

void createEasyRpcTcpServer()
{
    std::ofstream easyRpcTcpServerHeader("Server/EasyRpcTcpServer.h", std::ios::out);
    easyRpcTcpServerHeader << R"(#pragma once

#include <memory>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class EasyRpcSessionBase;

class EasyRpcTcpServer : public std::enable_shared_from_this<EasyRpcTcpServer>
{
    io_context   m_ioContext;
    tcp::socket   m_socket;
    tcp::acceptor m_acceptor;
    std::function<std::shared_ptr<EasyRpcSessionBase>()> m_creator;

    std::vector<std::shared_ptr<EasyRpcSessionBase>> m_sessions;

public:
    EasyRpcTcpServer(int port);

    void run();
    void onNewConnection(std::function<std::shared_ptr<EasyRpcSessionBase>()> creator);

private:
    void accept();
};
)";

    std::ofstream easyRpcTcpServerCpp("Server/EasyRpcTcpServer.cpp", std::ios::out);
    easyRpcTcpServerCpp << R"(#include "EasyRpcTcpServer.h"
#include "ServerSession.h"
#include "EasyRpcSessionBase.h"

EasyRpcTcpServer::EasyRpcTcpServer(int port) : m_ioContext(),
                                               m_socket(m_ioContext),
                                               m_acceptor( m_ioContext, tcp::endpoint(tcp::v4(), port))
{

}

void EasyRpcTcpServer::run()
{
    post( m_ioContext, [this] { accept(); } );
    m_ioContext.run();
}

void EasyRpcTcpServer::accept()
{
    m_acceptor.async_accept( [this] (boost::system::error_code ec, tcp::socket socket ) {
        if (!ec)
        {
            std::cout << "Connection established" << socket.remote_endpoint().address().to_string() << ": " << socket.remote_endpoint().port() << std::endl;
            auto session = m_creator();
            m_sessions.push_back(session);
            session->moveSocket(std::move(socket));
            session->readPacket();
        }
        accept();
    });
}

void EasyRpcTcpServer::onNewConnection(std::function<std::shared_ptr<EasyRpcSessionBase>()> creator)
{
    m_creator = creator;
}
)";
}

void createServerSession()
{
    std::ofstream serverSession("Server/ServerSession.h", std::ios::out);
    serverSession << R"DELIMITER(#pragma once

#include "EasyRpcFunction.h"

#include <iostream>
#include <boost/asio.hpp>

class ServerSession : public std::enable_shared_from_this<ServerSession>
{
    boost::asio::streambuf   m_streambuf;

public:
    boost::asio::io_context      m_ioContextUnused;
    boost::asio::ip::tcp::socket m_socket;

    ServerSession() : m_ioContextUnused(), m_socket(m_ioContextUnused) {}

    ~ServerSession() { std::cout << "!!!! ~ClientSession()" << std::endl; }

    virtual void onPacketReceived(std::shared_ptr<std::string> packet) = 0;

    void moveSocket(boost::asio::ip::tcp::socket&& socket)
    {
        m_socket = std::move(socket);
    }

    void readPacket()
    {
        auto packetSize = std::make_shared<uint32_t>(0);
        boost::asio::async_read(m_socket, boost::asio::buffer(packetSize.get(), sizeof(*packetSize)),
                                transfer_exactly(sizeof(*packetSize)),
                                [this, packetSize](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
            std::cout << "Async_read bytes transferred: " << bytes_transferred << std::endl;

            if (ec)
            {
                std::cerr << "Read packet error: " << ec.message() << std::endl;
            }
            if (*packetSize == 0)
            {
                std::cerr << "Bad packet" << std::endl;
                return;
            }

            std::shared_ptr<std::string> packet = std::make_shared<std::string>();
            packet->resize(*packetSize);

            boost::asio::async_read(m_socket, boost::asio::buffer(*packet),
                                    transfer_exactly(*packetSize),
                                    [this, packet](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {

                std::cout << "Async_read bytes transferred: " << bytes_transferred << std::endl;

                if (ec)
                {
                    std::cerr << "Read packet error: " << ec.message() << std::endl;
                }

                onPacketReceived(packet);
                readPacket();
            });
        });
    }

    void sendEnum(const uint32_t result)
    {
        boost::asio::write(m_socket, boost::asio::buffer(&result, sizeof(result)));
    }

    void sendPacket(const std::string& buffer)
    {
        uint32_t packetSize = buffer.size();
        boost::asio::write(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));

        boost::asio::write(m_socket, boost::asio::buffer(buffer.c_str(), packetSize));
    }

    void closeConnection()
    {
        boost::system::error_code ec;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec)
        {
            std::cerr << "Shutdown error: " << ec.message() << std::endl;
        }

        m_socket.close(ec);
        if (ec)
        {
            std::cerr << "Close error: " << ec.message() << std::endl;
        }
    }
};)DELIMITER";
}

void createServerFolder()
{
    if(std::filesystem::create_directory("Server"))
    {
        createEasyRpcTcpServer();
        createServerSession();
    }
    else
    {
        std::cerr << "Folder \"Server\" couldn't be created";
    }
}

void generateFiles(const std::vector<RPCFunction>& rpcFunctions)
{
    createEasyRpcFunction(rpcFunctions);
    createEasyRpcSessionBase(rpcFunctions);
    createEasyRpcClientBase(rpcFunctions);
    createClientFolder();
    createServerFolder();
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
