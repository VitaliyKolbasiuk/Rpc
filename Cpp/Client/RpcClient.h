#pragma once

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
};