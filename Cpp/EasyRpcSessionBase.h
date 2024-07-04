#pragma once

#include "Server/EasyRpcTcpServer.h"
#include "Server/ServerSession.h"
#include "EasyRpcFunction.h"

#include <cstdint>
#include <string>
#include <memory>

class EasyRpcSessionBase : public ServerSession
{
public:
    virtual ~EasyRpcSessionBase() = default;

    //::generateVirtualFunctions::
    virtual void on_plus(uint64_t context, double arg1, double arg2) = 0;
    virtual void on_minus(uint64_t context, double arg1, double arg2) = 0;
    //::generateVirtualFunctions::end::

    // Virtual
    void onPacketReceived(std::shared_ptr<std::string> packet) override
    {
        // Detect operation/call
        auto* ptr = const_cast<char*>(packet->c_str());
        const char* packetEnd = ptr + packet->size();

        uint16_t operation;
        readFromPacket(operation, &ptr, packetEnd);

        uint64_t context;
        readFromPacket(context, &ptr, packetEnd);

        //::generateSwitch::
        switch(operation)
        {
            case EasyRpcFunction::plus:
            {
                double arg1;
                readFromPacket(arg1, &ptr, packetEnd);

                double arg2;
                readFromPacket(arg2, &ptr, packetEnd);

                on_plus(context, arg1, arg2);
                break;
            }
            case EasyRpcFunction::minus:
            {
                double arg1;
                readFromPacket(arg1, &ptr, packetEnd);

                double arg2;
                readFromPacket(arg2, &ptr, packetEnd);

                on_minus(context, arg1, arg2);
                break;
            }
            default:
                closeConnection();
                break;
        }
        //::generateSwitch::end::
    }

    //::generateResponseFunction::
    void send_plus_response(const uint64_t context, const double value, const std::string& error_message)
    {
        if (error_message.size() > 0xffff)
        {
            throw std::runtime_error("Too long string in send_plus_response");
        }
        std::string buffer;

        buffer.resize(sizeof(context) + sizeof(value) + 2 + error_message.size());
        auto* ptr = const_cast<char*>(buffer.c_str());

        EasyRpcFunction operation = EasyRpcFunction::plus;
        write(operation, &ptr);
        write(context, &ptr);
        write(value, &ptr);
        write(error_message, &ptr);

        sendPacket(buffer);
    }

    //::generateResponseFunction::
    void send_minus_response(const uint64_t context, const double value, const std::string& error_message )
    {
        if (error_message.size() > 0xffff)
        {
            throw std::runtime_error("Too long string in send_plus_response");
        }
        std::string buffer;

        buffer.resize(sizeof(context) + sizeof(value) + 2 + error_message.size());
        auto* ptr = const_cast<char*>(buffer.c_str());

        EasyRpcFunction operation = EasyRpcFunction::minus;
        write(operation, &ptr);
        write(context, &ptr);
        write(value, &ptr);
        write(error_message, &ptr);

        sendPacket(buffer);
    }
    //::generateResponseFunction::end::

private:
    void readFromPacket(EasyRpcFunction& operation, char** ptr, const char* packetEnd)
    {
        if (packetEnd <= *ptr + sizeof(operation))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&operation, *ptr, sizeof(operation));
        *ptr += sizeof(operation);
    }

    void readFromPacket(uint64_t& context, char** ptr, const char* packetEnd)
    {
        if (packetEnd <= *ptr + sizeof(context))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&context, *ptr, sizeof(context));
        *ptr += sizeof(context);
    }

    //::generateReadFromPacket::
    void readFromPacket(double& value, char** ptr, const char* packetEnd)
    {
        if (packetEnd < *ptr + sizeof(value))
        {
            throw std::runtime_error("Buffer too small");
        }

        std::memcpy(&value, *ptr, sizeof(value));
        *ptr += sizeof(value);
    }
    //::generateReadFromPacket::end

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
    //::generateWrite::end

    //::generateWrite::
    void write(const std::string& str, char** ptr)
    {
        uint16_t length = str.size();

        std::memcpy(*ptr, &length, sizeof(length));
        *ptr += sizeof(length);

        std::memcpy(*ptr, str.c_str(), length);
        *ptr += length;
    }
    //::generateWrite::end
};