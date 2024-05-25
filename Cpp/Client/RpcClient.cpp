#include "Protobuf/AddressBook.pb.h"
#include "RpcClient.h"
#include "TcpClient.h"

#include <iostream>

RpcClient::RpcClient()
{

}

void RpcClient::setTcpClient(const std::weak_ptr<TcpClient> &tcpClient)
{
    m_tcpClient = tcpClient;
}

void RpcClient::onSocketConnected()
{
    std::cout << "Socket connected" << std::endl;

    //requestCalculation();
}

//void RpcClient::requestCalculation()
//{
//    double result;
//
//    std::cout << "Write operation and two numbers" << std::endl;
//    std::string operation;
//    int firstNum, secondNum;
//    std::cin >> operation >> firstNum >> secondNum;
//
//    Operations operations;
//    if (operation == "+")
//    {
//        operations = Operations::plus;
//    }
//    else if (operation == "-")
//    {
//        operations = Operations::minus;
//    }
//    else if (operation == "*")
//    {
//        operations = Operations::multiply;
//    }
//    else if (operation == "/")
//    {
//        operations = Operations::divide;
//    }
//    else
//    {
//        requestCalculation();
//    }
//
//    if (std::string error = calculate(operations, firstNum, secondNum, result); !error.empty())
//    {
//        std::cerr << "ERROR " << error << std::endl;
//        return;
//    }
//    std::cout << "Received result: " << result << std::endl;
//
//    requestCalculation();
//}

void RpcClient::sendProtobufer(const google::protobuf::MessageLite& messageLite)
{
    std::string buffer;
    if (!messageLite.SerializeToString(&buffer))
    {
        std::cout << "Failed to parse protobuf";
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
                        std::cout << "Result: Failed to parse protobuf";
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
                        std::cout << "FailResult: Failed to parse protobuf";
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


