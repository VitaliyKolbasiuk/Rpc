#include "TcpServer.h"
#include "ServerSession.h"

TcpServer::TcpServer(io_context& ioContext, IRpcModel& rpcModel, int port) : m_ioContext(ioContext),
                                                        m_rpcModel(rpcModel),
                                                        m_socket(m_ioContext),
                                                        m_acceptor( m_ioContext, tcp::endpoint(tcp::v4(), port))
{

}

void TcpServer::execute()
{
    post( m_ioContext, [this] { accept(); } );
    m_ioContext.run();
}

void TcpServer::accept()
{
    m_acceptor.async_accept( [this] (boost::system::error_code ec, tcp::socket socket ) {
        if (!ec)
        {
            std::cout << "Connection established" << socket.remote_endpoint().address().to_string() << ": " << socket.remote_endpoint().port() << std::endl;
            auto session = std::make_shared<ServerSession>(m_ioContext, std::move(socket), m_rpcModel, weak_from_this());
            m_sessions.push_back(session);
            session->readPacket();
        }
        accept();
    });
}
