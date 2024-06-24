#include "EasyRpcClientBase.h"
#include "RpcClient.h"

int main()
{
    RpcClient rpcClient;
    rpcClient.start("127.0.0.1", 12345);

    if (rpcClient.connectionError())
    {
        std::cerr << "Connection failed: " << rpcClient.connectionError().message() << std::endl;
        return -1;
    }

    rpcClient.plus(1, 2, [](double value, std::string error_message){
        if (error_message.empty())
        {
            std::cout << "1 + 2 = " << value << std::endl;
        }
    });

    rpcClient.minus(8, 3, [](double value, std::string error_message){
        if (error_message.empty())
        {
            std::cout << "8 - 3 = " << value << std::endl;
        }
    });

    rpcClient.wait();
}