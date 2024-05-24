#pragma once

class ITcpClient
{
public:
    virtual ~ITcpClient() = default;

public:
    virtual void onSocketConnected() = 0;
    virtual void closeConnection() = 0;
};
