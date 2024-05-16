#pragma once

#include "ClientInterfaces.h"
#include <boost/asio.hpp>
#include <QDebug>
#include <utility>

using namespace boost::asio;
using ip::tcp;

class TcpClient
{
    io_context&  m_ioContext;
    tcp::socket m_socket;
    std::shared_ptr<IRpcClient> m_client;

public:
    TcpClient( io_context&  ioContext, std::shared_ptr<IRpcClient> client) :
            m_ioContext(ioContext),
            m_socket(m_ioContext),
            m_client(std::move(client))
    {}

    ~TcpClient()
    {
        qCritical() << "!!!! ~Client(): ";
    }

    void connect(const std::string& addr, const int& port)
    {
        qDebug() << "Connect: " << addr << ' ' << port;
        auto endpoint = tcp::endpoint(ip::address::from_string( addr.c_str()), port);

        m_socket.async_connect(endpoint, [this] (const boost::system::error_code& error)
        {
            if ( error )
            {
                qCritical() <<"Connection error: " << error.message();
            }
            else
            {
                qDebug() << "Connection established";
                m_client->onSocketConnected();
            }
        });
    }

    void sendPacket()
    {
//        qDebug() << "Send Packet buffer packetLength: " << packet.packetLength() << " Type: " << gTypeMap.m_typeMap[packet.packetType()];
//        async_write(m_socket, boost::asio::buffer(&packet, sizeof(PacketHeader<T>)),
//                    [this] (const boost::system::error_code& ec, std::size_t bytes_transferred ) {
//                        qDebug() << "Async_write bytes transferred: " << bytes_transferred;
//                        if ( ec )
//                        {
//                            qCritical() << "!!!! Session::sendMessage error (1): " << ec.message();
//                            exit(-1);
//                        }
//                    });
    }

};