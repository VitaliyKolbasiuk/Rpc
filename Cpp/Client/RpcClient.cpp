#include "Protobuf/AddressBook.pb.h"
#include "RpcClient.h"
#include "TcpClient.h"

#include <iostream>

RpcClient::RpcClient() : EasyRpcClientBase()
{

}

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



void RpcClient::closeConnection()
{

}


