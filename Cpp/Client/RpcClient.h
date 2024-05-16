#pragma once

#include "ClientInterfaces.h"

class TcpClient;
class RpcClient : public IRpcClient
{

    std::weak_ptr<TcpClient> m_tcpClient;
public:
    RpcClient();

    void setTcpClient(const std::weak_ptr<TcpClient>& tcpClient);
    void onSocketConnected() override;
    void closeConnection() override;
};
