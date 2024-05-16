#include "../cmake-build-debug/Protobuf/AddressBook.pb.h"
#include "RpcClient.h"

#include <QDebug>

RpcClient::RpcClient()
{

}

void RpcClient::setTcpClient(const std::weak_ptr<TcpClient> &tcpClient)
{
    m_tcpClient = tcpClient;
}
void RpcClient::onSocketConnected()
{
    qDebug() << "Socket connected";
}

void RpcClient::closeConnection()
{

}


