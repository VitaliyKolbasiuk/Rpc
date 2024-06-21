#pragma once

#include "Interfaces.h"
#include "ModernRpcFunction.h"

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

    virtual void onPacketReceived(std::shared_ptr<std::string> packet) = 0;

    void moveSocket(boost::asio::ip::tcp::socket&& socket)
    {
        m_socket = std::move(socket);
    }

    void readPacket()
    {
        auto packetSize = std::make_shared<uint32_t>(0);
        boost::asio::async_read(m_socket, boost::asio::buffer(packetSize.get(), sizeof(*packetSize)),
                                transfer_exactly(sizeof(*packetSize)),
                                [this, packetSize](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
            std::cout << "Async_read bytes transferred: " << bytes_transferred << std::endl;

            if (ec)
            {
                std::cerr << "Read packet error: " << ec.message() << std::endl;
            }
            if (*packetSize == 0)
            {
                std::cerr << "Bad packet" << std::endl;
                return;
            }

            std::shared_ptr<std::string> packet = std::make_shared<std::string>();
            packet->resize(*packetSize);

            boost::asio::async_read(m_socket, boost::asio::buffer(*packet),
                                    transfer_exactly(*packetSize),
                                    [this, packet](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {

                std::cout << "Async_read bytes transferred: " << bytes_transferred << std::endl;

                if (ec)
                {
                    std::cerr << "Read packet error: " << ec.message() << std::endl;
                }

                onPacketReceived(packet);
                readPacket();
            });
        });
    }

    void sendEnum(const uint32_t result)
    {
        boost::asio::write(m_socket, boost::asio::buffer(&result, sizeof(result)));
    }

    void sendPacket(const std::string& buffer)
    {
        uint32_t packetSize = buffer.size();
        boost::asio::write(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));

        boost::asio::write(m_socket, boost::asio::buffer(buffer.c_str(), packetSize));
    }
};
