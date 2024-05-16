#include "RpcServer.h"
#include "ServerSession.h"

#include <QDebug>

RpcServer::RpcServer(io_context& ioContext, int port) : m_ioContext(ioContext),
                                                                     m_socket(m_ioContext),
                                                                     m_acceptor( m_ioContext, tcp::endpoint(tcp::v4(), port))
{

}

void RpcServer::execute()
{
    post( m_ioContext, [this] { accept(); } );
    m_ioContext.run();
}

void RpcServer::accept()
{
    m_acceptor.async_accept( [this] (boost::system::error_code ec, tcp::socket socket ) {
        if (!ec)
        {
            qDebug() << "Connection established" << socket.remote_endpoint().address().to_string() << ": " << socket.remote_endpoint().port();
            auto session = std::make_shared<ServerSession>(m_ioContext, std::move(socket), weak_from_this());
            m_sessions.push_back(session);
            session->readPacket();
        }
        accept();
    });
}
