#pragma once

#include "ModernRpcFunction.h"
#include "ModernRpcClientBase.h"

#include <memory>

class TcpClient;
class RpcClient : public ModernRpcClientBase
{
//    std::weak_ptr<TcpClient> m_tcpClient;
public:
    RpcClient(boost::asio::io_context& context);

//    void setTcpClient(const std::weak_ptr<TcpClient>& tcpClient);
    void onSocketConnected() override;
    //void requestCalculation();
    void closeConnection();
};

