#include "ModernRpcTcpServer.h"
#include "ServerSession.h"
#include "ModernRpc.h"

ModernRpcTcpServer::ModernRpcTcpServer(io_context& ioContext, int port) : m_ioContext(ioContext),
                                                                                               m_socket(m_ioContext),
                                                                                               m_acceptor( m_ioContext, tcp::endpoint(tcp::v4(), port))
{

}

void ModernRpcTcpServer::run()
{
    post( m_ioContext, [this] { accept(); } );
    m_ioContext.run();
}

void ModernRpcTcpServer::accept()
{
    m_acceptor.async_accept( [this] (boost::system::error_code ec, tcp::socket socket ) {
        if (!ec)
        {
            std::cout << "Connection established" << socket.remote_endpoint().address().to_string() << ": " << socket.remote_endpoint().port() << std::endl;
            //auto session = std::make_shared<ServerSession>(m_ioContext, std::move(socket), m_rpcModel, weak_from_this());
            auto session = createModernRpcSessionBase();
            m_sessions.push_back(session);
            session->moveSocket(std::move(socket));
            session->readPacket();
        }
        accept();
    });
}
