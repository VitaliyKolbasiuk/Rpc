#pragma once

#include "../cmake-build-debug/Protobuf/AddressBook.pb.h"

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
    std::shared_ptr<ITcpClient> m_client;

public:
    TcpClient( io_context&  ioContext, std::shared_ptr<ITcpClient> client) :
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

    void sendPacket(const std::string& buffer)
    {
        //qDebug() << "HelloWorld size: " << sizeof(packet);
        //qDebug() << "Message size: " << packet.message().size();

        uint32_t packetSize = buffer.size();
        boost::asio::write(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));

        boost::asio::write(m_socket, boost::asio::buffer(buffer.c_str(), packetSize));
    }

    void sendEnum(const uint32_t operation)
    {
        boost::asio::write(m_socket, boost::asio::buffer(&operation, sizeof(operation)));
    }

    uint32_t readEnum()
    {
        uint32_t result;
        boost::asio::read(m_socket, boost::asio::buffer(&result, sizeof(result)));
        return result;
    }

    uint8_t* readPacket(uint32_t& packetSize)
    {
        // Read arguments
        boost::asio::read(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));

        if (packetSize == 0)
        {
            qDebug() << "Bad packet";
            return nullptr;
        }

        uint8_t* buffer = new uint8_t[packetSize];
        boost::asio::read(m_socket, boost::asio::buffer(buffer, packetSize));

        return buffer;
    }

};