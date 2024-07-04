#pragma once

#include <memory>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class EasyRpcSessionBase;

class EasyRpcTcpServer : public std::enable_shared_from_this<EasyRpcTcpServer>
{
    io_context   m_ioContext;
    tcp::socket   m_socket;
    tcp::acceptor m_acceptor;
    std::function<std::shared_ptr<EasyRpcSessionBase>()> m_creator;

    std::vector<std::shared_ptr<EasyRpcSessionBase>> m_sessions;

public:
    EasyRpcTcpServer(int port);

    void run();
    void onNewConnection(std::function<std::shared_ptr<EasyRpcSessionBase>()> creator);

private:
    void accept();
};
