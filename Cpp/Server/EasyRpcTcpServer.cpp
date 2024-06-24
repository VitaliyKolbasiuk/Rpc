#include "EasyRpcTcpServer.h"
#include "ServerSession.h"
#include "EasyRpcSessionBase.h"

EasyRpcTcpServer::EasyRpcTcpServer(int port) : m_ioContext(),
                                               m_socket(m_ioContext),
                                               m_acceptor( m_ioContext, tcp::endpoint(tcp::v4(), port))
{

}

void EasyRpcTcpServer::run()
{
    post( m_ioContext, [this] { accept(); } );
    m_ioContext.run();
}

void EasyRpcTcpServer::accept()
{
    m_acceptor.async_accept( [this] (boost::system::error_code ec, tcp::socket socket ) {
        if (!ec)
        {
            std::cout << "Connection established" << socket.remote_endpoint().address().to_string() << ": " << socket.remote_endpoint().port() << std::endl;
            //auto session = std::make_shared<ServerSession>(m_ioContext, std::move(socket), m_rpcModel, weak_from_this());
            auto session = m_creator();
            m_sessions.push_back(session);
            session->moveSocket(std::move(socket));
            session->readPacket();
        }
        accept();
    });
}

void EasyRpcTcpServer::onNewConnection(std::function<std::shared_ptr<EasyRpcSessionBase>()> creator)
{
    m_creator = creator;
}
