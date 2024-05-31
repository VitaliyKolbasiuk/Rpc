#pragma once

#include "Server/ModernRpcTcpServer.h"
#include "Server/ServerSession.h"
#include <cstdint>
#include <string>
#include <memory>

enum ModernRpcFunction: uint32_t
{
    plus = 0,
    minus
};

class ModernRpcClientBase
{
public:
    virtual ~ModernRpcClientBase() = default;

    void plus( double arg1, double arg2 );
    void minus( double arg1, double arg2 );

    virtual void on_plus_result( double value, std::string error_message ) = 0;
    virtual void on_minus_result( double value, std::string error_message ) = 0;
};

class ModernRpcSessionBase : public ServerSession
{

public:
    virtual ~ModernRpcSessionBase() = default;

    virtual void on_plus(uint64_t context, double arg1, double arg2 ) = 0;
    virtual void on_minus(uint64_t context, double arg1, double arg2 ) = 0;

    void send_plus_response(uint64_t context, double value, std::string error_message );
    void send_minus_response(uint64_t context, double value, std::string error_message );
};

// Function will be called after accepting the connection to the client socket
std::shared_ptr<ModernRpcSessionBase> createModernRpcSessionBase();

inline void startModernRpcServer(int portNum)
{
    boost::asio::io_context ioContext;
    ModernRpcTcpServer tcpServer(ioContext, portNum);
    tcpServer.run();
}