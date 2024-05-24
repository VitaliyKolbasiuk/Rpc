#pragma once

#include "Interfaces.h"
#include "../Operations.h"

#include <QDebug>
#include <boost/asio.hpp>
#include <map>


class ServerSession : public std::enable_shared_from_this<ServerSession>
{
    boost::asio::io_context& m_ioContext;
    boost::asio::streambuf   m_streambuf;
    std::weak_ptr<IServer>   m_tcpServer;
    IRpcModel&               m_rpcModel;

public:
    boost::asio::ip::tcp::socket              m_socket;

    ServerSession(boost::asio::io_context& ioContext, boost::asio::ip::tcp::socket&& socket, IRpcModel& rpcModel, std::weak_ptr<IServer> tcpServer)
            : m_ioContext(ioContext),
              m_rpcModel(rpcModel),
              m_tcpServer(tcpServer),
              m_socket(std::move(socket))
    {
        if (auto tcpServerPtr = m_tcpServer.lock(); tcpServerPtr)
        {
            qDebug() << "TcpServer lock";
        }
        //async_read( m_socket );
    }

    ~ServerSession() { qCritical() << "!!!! ~ClientSession()"; }

    void readPacket()
    {
        uint32_t operation;
        boost::asio::read(m_socket, boost::asio::buffer(&operation, sizeof(operation)));

        qDebug() << "Operation: " << operation;

        // Read arguments
        uint32_t packetSize = 0;
        boost::asio::read(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));

        if (packetSize == 0)
        {
            qDebug() << "Bad packet";
            return;
        }

        uint8_t* bufferArguments = new uint8_t[packetSize];
        boost::asio::read(m_socket, boost::asio::buffer(bufferArguments, packetSize));

        Arguments2 arguments2;
        if (!arguments2.ParseFromArray(bufferArguments, packetSize))
        {
            qDebug() << "Failed to parse protobuf";
            return;
        }

        qDebug() << "Arguments: " << arguments2.arg1() << arguments2.arg2();

        m_rpcModel.calculate(operation, arguments2.arg1(), arguments2.arg2(), weak_from_this());
    }

    void sendEnum(const uint32_t result)
    {
        boost::asio::write(m_socket, boost::asio::buffer(&result, sizeof(result)));
    }

    void sendProtobuf(const google::protobuf::MessageLite& messageLite)
    {
        std::string buffer;
        if (!messageLite.SerializeToString(&buffer))
        {
            qDebug() << "Failed to parse protobuf";
        }

        sendPacket(buffer);
    }

    void sendPacket(const std::string& buffer)
    {
        uint32_t packetSize = buffer.size();
        boost::asio::write(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));

        boost::asio::write(m_socket, boost::asio::buffer(buffer.c_str(), packetSize));
    }
};
