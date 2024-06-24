#include "EasyRpcSessionBase.h"

class RpcSession : public EasyRpcSessionBase
{
public:
    RpcSession() {}
    virtual ~RpcSession() = default;

    // virtual
    void on_plus(uint64_t context, double arg1, double arg2) override
    {
        send_plus_response( context, arg1 + arg2, "");
    }

    // virtual
    void on_minus(uint64_t context, double arg1, double arg2 ) override
    {
        send_minus_response(context, arg1 - arg2, "");
    }
};


int main()
{
    EasyRpcTcpServer tcpServer(12345);
    tcpServer.onNewConnection([](){ return std::make_shared<RpcSession>(); });
    tcpServer.run();
}