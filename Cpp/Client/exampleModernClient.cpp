#include "ModernRpc.h"

class RpcClient : public ModernRpcClientBase
{
public:
    RpcClient () {}
    virtual ~RpcClient() = default;

    virtual void on_plus_result( double value, std::string error_message )
    {

    }
    virtual void on_minus_result( double value, std::string error_message ) override
    {

    }
};

int main()
{
    RpcClient rpcClient;
    rpcClient.plus(1, 2, [](double value, std::string error_message){

    });
    rpcClient.minus(8, 4);
}