#pragma once

#include "ClientInterfaces.h"
#include "Operations.h"

#include <memory>

class TcpClient;
class RpcClient : public ITcpClient
{

    std::weak_ptr<TcpClient> m_tcpClient;
public:
    RpcClient();

    void setTcpClient(const std::weak_ptr<TcpClient>& tcpClient);
    void onSocketConnected() override;
    //void requestCalculation();
    void sendProtobufer(const google::protobuf::MessageLite& messageLite);
    void closeConnection() override;
    std::string calculate(const Operations operation, const double arg1, const double arg2, double& outResult);
};

