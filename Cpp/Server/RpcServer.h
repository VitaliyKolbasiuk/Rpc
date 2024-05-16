#pragma once

#include "../cmake-build-debug/Protobuf/AddressBook.pb.h"
#include "Interfaces.h"

#include <memory>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class RpcServer : public IServer, public std::enable_shared_from_this<RpcServer>
{
   io_context&        m_ioContext;
   tcp::socket           m_socket;
   tcp::acceptor m_acceptor;

    std::vector<std::shared_ptr<ServerSession>> m_sessions;

public:
    RpcServer(boost::asio::io_context& ioContext, int port);

    void execute() override;
    void accept();
};
