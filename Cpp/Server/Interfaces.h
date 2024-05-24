#pragma once


class ServerSession;

class IServer
{
public:
    virtual ~IServer() = default;

    virtual void execute() = 0;
};

class IRpcModel
{
public:
    virtual ~IRpcModel() = default;

    virtual void calculate(const uint32_t operation, const double arg1, const double arg2, std::weak_ptr<ServerSession> session) = 0;
};