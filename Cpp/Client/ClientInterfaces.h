#pragma once

class IRpcClient
{
public:
    virtual ~IRpcClient() = default;

public:
    virtual void onSocketConnected() = 0;
    virtual void closeConnection() = 0;
};
