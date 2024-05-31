#pragma once

#include "Interfaces.h"
#include "../Operations.h"

#include <iostream>
#include <boost/asio.hpp>
#include <map>


class ServerSession : public std::enable_shared_from_this<ServerSession>
{
    boost::asio::streambuf   m_streambuf;

public:
    boost::asio::io_context      m_ioContextUnused;
    boost::asio::ip::tcp::socket m_socket;

    ServerSession() : m_ioContextUnused(), m_socket(m_ioContextUnused) {}

    ~ServerSession() { std::cout << "!!!! ~ClientSession()" << std::endl; }

    void moveSocket(boost::asio::ip::tcp::socket&& socket)
    {
        m_socket = std::move(socket);
    }

    void readPacket()
    {
        uint32_t operation;
        boost::asio::read(m_socket, boost::asio::buffer(&operation, sizeof(operation)));

        std::cout << "Operation: " << operation << std::endl;

        // Read arguments
        uint32_t packetSize = 0;
        boost::asio::read(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));

        if (packetSize == 0)
        {
            std::cerr << "Bad packet" << std::endl;
            return;
        }

        uint8_t* bufferArguments = new uint8_t[packetSize];
        boost::asio::read(m_socket, boost::asio::buffer(bufferArguments, packetSize));

        Arguments2 arguments2;
        if (!arguments2.ParseFromArray(bufferArguments, packetSize))
        {
            std::cerr << "Failed to parse protobuf" << std::endl;
            return;
        }

        std::cout << "Arguments: " << arguments2.arg1() << arguments2.arg2() << std::endl;

        //m_rpcModel.calculate(operation, arguments2.arg1(), arguments2.arg2(), weak_from_this());

        readPacket();
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
            std::cerr << "Failed to parse protobuf" << std::endl;
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
