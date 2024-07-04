#pragma once

#include "Client/TcpClient.h"
#include "EasyRpcFunction.h"

#include <cstdint>
#include <string>
#include <map>

class EasyRpcClientBase : public TcpClient
{
    //::generateMembers::
    uint64_t m_plus_context = 0;
    std::map<uint64_t, std::function<void(double value, std::string error_message)>> m_plus_map;
    std::mutex m_plus_map_mutex;

    uint64_t m_minus_context = 0;
    std::map<uint64_t, std::function<void(double value, std::string error_message)>> m_minus_map;
    std::mutex m_minus_map_mutex;
    //::generateMembers::end::

public:
    EasyRpcClientBase() {}
    virtual ~EasyRpcClientBase() = default;

    // Virtual
    void onPacketReceived(uint8_t* packet, const uint32_t packetSize) override
    {
        uint8_t* ptr = packet;

        EasyRpcFunction operation;
        readFromPacket(operation, ptr, packetSize);

        uint64_t context;
        readFromPacket(context, ptr, packetSize);

        //::generateSwitch::
        switch(operation)
        {
            case EasyRpcFunction::plus:
            {
                double value;
                readFromPacket(value, ptr, packetSize);

                std::string error_message;
                readFromPacket(error_message, ptr, packetSize);

                m_plus_map[context](value, error_message);
                break;
            }
            case EasyRpcFunction::minus:
            {
                double value;
                readFromPacket(value, ptr, packetSize);

                std::string error_message;
                readFromPacket(error_message, ptr, packetSize);

                m_minus_map[context](value, error_message);
                break;
            }
        }
        //::generateSwitch::end::
        delete[] packet;
    }

    //::generateFunctions::
    void plus( double arg1, double arg2, std::function<void(double value, std::string error_message)> func )
    {
        {
            std::lock_guard<std::mutex> lock_guard(m_plus_map_mutex);

            ++m_plus_context;
            m_plus_map[m_plus_context] = func;
        }

        std::string buffer;
        EasyRpcFunction operation = EasyRpcFunction::plus;

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
        EasyRpcFunction operation = EasyRpcFunction::minus;

        buffer.resize(sizeof(operation) + sizeof(m_minus_context) + sizeof(arg1) + sizeof(arg2));
        auto* ptr = const_cast<char*>(buffer.c_str());
        write(operation, &ptr);
        write(m_minus_context, &ptr);
        write(arg1, &ptr);
        write(arg2, &ptr);

        sendPacket(buffer);
    }
    //::generateFunctions::end

private:

    void readFromPacket(EasyRpcFunction& operation, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(operation))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&operation, ptr, sizeof(operation));
        ptr += sizeof(operation);
    }

    void readFromPacket(uint64_t& context, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(context))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&context, ptr, sizeof(context));
        ptr += sizeof(context);
    }

    //::generateReadFromPacket::
    void readFromPacket(double& value, uint8_t*& ptr, const uint32_t packetSize)
    {
        if (ptr + packetSize <= ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, ptr, sizeof(value));
        ptr += sizeof(value);
    }

    void readFromPacket(std::string& message, uint8_t*& ptr, const uint32_t packetSize)
    {
        uint16_t length;

        if (ptr + packetSize <= ptr + sizeof(length))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&length, ptr, sizeof(length));
        ptr += sizeof(length);

        if (ptr + packetSize <= ptr + length)
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&message[0], ptr, length);
        ptr += length;
    }
    //::generateReadFromPacket::end::

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

    //::generateWrite::
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
    //::generateWrite::end::
};