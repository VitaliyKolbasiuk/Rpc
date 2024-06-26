#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <utility>

using namespace boost::asio;
using ip::tcp;

class TcpClient
{
protected:
    io_context  m_ioContext;

private:
    tcp::socket m_socket;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_workGuard;
public:

    TcpClient() :
            m_ioContext(),
            m_socket(m_ioContext),
            m_workGuard(boost::asio::make_work_guard(m_ioContext))
    {}

    ~TcpClient()
    {
        std::cout << "!!!! ~Client(): " << std::endl;
    }

    virtual void onPacketReceived(uint8_t* packet, const uint32_t packetSize) = 0;
    virtual void onSocketConnected() {}

    void connect(const std::string& addr, const int& port, std::promise<boost::system::error_code>& promise)
    {
        std::cout << "Connect: " << addr << ' ' << port << std::endl;
        auto endpoint = tcp::endpoint(ip::address::from_string( addr.c_str()), port);

        m_socket.async_connect(endpoint, [this, &promise] (const boost::system::error_code& error)
        {
            if ( error )
            {
                std::cerr <<"Connection error: " << error.message() << std::endl;
                promise.set_value(error);
            }
            else
            {
                std::cout << "Connection established" << std::endl;
                onSocketConnected();
                promise.set_value(error);
                readPacket();
            }
        });

    }

    void run()
    {
        m_ioContext.run();
    }

    std::mutex m_mutex;
    void sendPacket(const std::string& buffer)
    {
        uint32_t packetSize = buffer.size();

        std::lock_guard<std::mutex> lock_guard(m_mutex);
        boost::asio::write(m_socket, boost::asio::buffer(&packetSize, sizeof(packetSize)));
        boost::asio::write(m_socket, boost::asio::buffer(buffer.c_str(), packetSize));
    }

//    void sendEnum(const uint32_t operation)
//    {
//        boost::asio::write(m_socket, boost::asio::buffer(&operation, sizeof(operation)));
//    }
//
//    uint32_t readEnum()
//    {
//        uint32_t result;
//        boost::asio::read(m_socket, boost::asio::buffer(&result, sizeof(result)));
//        return result;
//    }

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

                uint8_t* packet = new uint8_t[*packetSize];
                boost::asio::async_read(m_socket, boost::asio::buffer(packet, *packetSize), transfer_exactly(*packetSize),
                                        [this, packet, packetSize](const boost::system::error_code& ec, std::size_t bytes_transferred)
                  {
                      std::cout << "Async_read bytes transferred: " << bytes_transferred << std::endl;

                      if (ec)
                      {
                          std::cerr << "Read packet error: " << ec.message() << std::endl;
                      }

                      onPacketReceived(packet, *packetSize);
                      readPacket();
                  });
            });
    }
};