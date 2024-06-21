#include "ModernRpcSessionBase.h"

class RpcSession : public ModernRpcSessionBase
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

std::shared_ptr<ModernRpcSessionBase> createModernRpcSessionBase()
{
    return std::make_shared<RpcSession>();
}

int main()
{
    startModernRpcServer(12345);
}