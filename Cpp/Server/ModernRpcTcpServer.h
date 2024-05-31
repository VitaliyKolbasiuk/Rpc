#pragma once

#include "Protobuf/AddressBook.pb.h"
#include "Interfaces.h"

#include <memory>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class ModernRpcSessionBase;

class ModernRpcTcpServer : public IServer, public std::enable_shared_from_this<ModernRpcTcpServer>
{
    //IRpcModel&    m_rpcModel;
    io_context&   m_ioContext;
    tcp::socket   m_socket;
    tcp::acceptor m_acceptor;

    std::vector<std::shared_ptr<ModernRpcSessionBase>> m_sessions;

public:
    ModernRpcTcpServer(boost::asio::io_context& ioContext, int port);

    void run() override;
    void accept();
};
