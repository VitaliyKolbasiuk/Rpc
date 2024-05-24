#include "../Operations.h"
#include "../cmake-build-debug/Protobuf/AddressBook.pb.h"
#include "../cmake-build-debug/Protobuf/AddressBook.pb.cc"
#include "RpcClient.h"
#include "TcpClient.h"

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

    double result;
    if (std::string error = calculate(Operations::plus, 5, 10, result); !error.empty())
    {
        qDebug() << "ERROR " << error;
        return;
    }
    qDebug() << "Received result: " << result;

}

void RpcClient::sendProtobufer(const google::protobuf::MessageLite& messageLite)
{
    std::string buffer;
    if (!messageLite.SerializeToString(&buffer))
    {
        qDebug() << "Failed to parse protobuf";
    }

    if (auto ptr = m_tcpClient.lock(); ptr)
    {
        ptr->sendPacket(buffer);
    }
}

std::string RpcClient::calculate(const Operations operation, const double arg1, const double arg2, double &outResult)
{
    if (auto ptr = m_tcpClient.lock(); ptr)
    {
        ptr->sendEnum(operation);
    }

    Arguments2 arguments2;
    arguments2.set_arg1(arg1);
    arguments2.set_arg2(arg2);

    sendProtobufer(arguments2);

    // Receive result
    uint32_t successOrFail;

    if (auto ptr = m_tcpClient.lock(); ptr)
    {
        successOrFail = ptr->readEnum();
    }

    if (auto ptr = m_tcpClient.lock(); ptr)
    {
        uint32_t packetSize;
        uint8_t* buffer = ptr->readPacket(packetSize);
        if (buffer != nullptr)
        {
            switch (successOrFail)
            {
                case Results::success:
                {
                    Result result;
                    if (!result.ParseFromArray(buffer, packetSize))
                    {
                        qDebug() << "Result: Failed to parse protobuf";
                        break;
                    }
                    outResult = result.value();
                    break;
                }
                case Results::fail:
                {
                    FailResult failResult;
                    if (!failResult.ParseFromArray(buffer, packetSize))
                    {
                        qDebug() << "FailResult: Failed to parse protobuf";
                        break;
                    }
                    return failResult.error_message();
                }
            }
        }
    }
    return "";
}

void RpcClient::closeConnection()
{

}


