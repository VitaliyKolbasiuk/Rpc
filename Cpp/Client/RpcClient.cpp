#include "Protobuf/AddressBook.pb.h"
#include "RpcClient.h"
#include "TcpClient.h"

#include <iostream>

RpcClient::RpcClient(boost::asio::io_context& context) : ModernRpcClientBase(context)
{

}

//void RpcClient::setTcpClient(const std::weak_ptr<TcpClient> &tcpClient)
//{
//    //m_tcpClient = tcpClient;
//}

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


void RpcClient::closeConnection()
{

}


