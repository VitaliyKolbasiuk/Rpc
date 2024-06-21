#pragma once

#include "Client/TcpClient.h"
#include "ModernRpcFunction.h"

#include <cstdint>
#include <string>
#include <memory>
#include <map>

class ModernRpcClientBase : public TcpClient
{
    uint64_t m_plus_context = 0;
    std::map<uint64_t, std::function<void(double value, std::string error_message)>> m_plus_map;
    std::mutex m_plus_map_mutex;

    uint64_t m_minus_context = 0;
    std::map<uint64_t, std::function<void(double value, std::string error_message)>> m_minus_map;
    std::mutex m_minus_map_mutex;
    boost::asio::io_context m_context;
public:
    ModernRpcClientBase(boost::asio::io_context& context) : TcpClient(context) {}
    virtual ~ModernRpcClientBase() = default;

    // Virtual
    void onPacketReceived(uint8_t* packet, const uint32_t packetSize) override
    {
        uint8_t* ptr = packet;

        uint16_t operation;
        readFromPacket(operation, ptr, packetSize);

        uint64_t context;
        readFromPacket(context, ptr, packetSize);

        switch(operation)
        {
            case ModernRpcFunction::plus:
            {
                double value;
                readFromPacket(value, ptr, packetSize);

                std::string error_message;
                ptrdiff_t ptrDiff = ptr - packet;
                readFromPacket(error_message, ptr, packetSize - ptrDiff);

                m_plus_map[context](value, error_message);
                break;
            }
            case ModernRpcFunction::minus:
            {
                double value;
                readFromPacket(value, ptr, packetSize);

                std::string error_message;
                ptrdiff_t ptrDiff = ptr - packet;
                readFromPacket(error_message, ptr, packetSize - ptrDiff);

                m_minus_map[context](value, error_message);
                break;
            }
        }

        delete packet;
    }

    void plus( double arg1, double arg2, std::function<void(double value, std::string error_message)> func )
    {
        {
            std::lock_guard<std::mutex> lock_guard(m_plus_map_mutex);

            ++m_plus_context;
            m_plus_map[m_plus_context] = func;
        }

        std::string buffer;
        enum ModernRpcFunction operation = ModernRpcFunction::plus;

        buffer.resize(sizeof(operation) + sizeof(m_plus_context) + sizeof(arg1) + sizeof(arg2));
        auto* ptr = const_cast<char*>(buffer.c_str());
        write(operation, &ptr);
        write(m_plus_context, &ptr);
        write(arg1, &ptr);
        write(arg2, &ptr);

        sendPacket(buffer);
    }

    void minus( double arg1, double arg2, std::function<void(double value, std::string error_message)> func )
    {
        {
            std::lock_guard<std::mutex> lock_guard(m_minus_map_mutex);

            ++m_minus_context;
            m_minus_map[m_minus_context] = func;
        }

        std::string buffer;
        enum ModernRpcFunction operation = ModernRpcFunction::minus;

        buffer.resize(sizeof(operation) + sizeof(m_minus_context) + sizeof(arg1) + sizeof(arg2));
        auto* ptr = const_cast<char*>(buffer.c_str());
        write(operation, &ptr);
        write(m_minus_context, &ptr);
        write(arg1, &ptr);
        write(arg2, &ptr);

        sendPacket(buffer);
    }

private:

    void readFromPacket(std::string& message, uint8_t*& ptr, const uint32_t bytesLeft) // how to read it?
    {
        std::memcpy(&message[0], ptr, bytesLeft);
        ptr += sizeof(bytesLeft);
    }

    void readFromPacket(uint16_t& value, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, ptr, sizeof(value));
        ptr += sizeof(value);
    }

    void readFromPacket(uint64_t& value, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, ptr, sizeof(value));
        ptr += sizeof(value);
    }

    void readFromPacket(double& value, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, ptr, sizeof(value));
        ptr += sizeof(value);
    }

    void write(const uint16_t& value, char** ptr)
    {
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);
    }

    void write(const uint64_t& value, char** ptr)
    {
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);
    }

    void write(const double& value, char** ptr)
    {
        std::memcpy(*ptr, &value, sizeof(value));
        *ptr += sizeof(value);
    }

    void write(const std::string& str, char** ptr)
    {
        uint16_t length = str.size();

        std::memcpy(*ptr, &length, sizeof(length));
        *ptr += sizeof(length);

        std::memcpy(*ptr, str.c_str(), length);
        *ptr += length;
    }
};

//inline void startModernClient()
//{
//    boost::asio::io_context ioContext;
//    TcpClient tcpClient(ioContext, );
//    tcpServer.run();
//}