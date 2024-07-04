EasyRpc is a framework for efficient communication between distributed systems. It allows client applications to call methods of remote services as if they were local, hiding the complexity of network communication. To the client, it looks like calling local methods.
In EasyRpc, data transfer between client and server is done in binary format, which is more compact and fast compared to JSON or XML. The structure of the transferred data is described by the user in a special meta-file with the following format:
rpc (argument-list) call-name(argument-list) 
notification (argument-list) notification-name(argument-list)
The argument-list is a list of pairs: {type name}, where name may not exist.
rpc - a message that a client sends to the server and receives a response.
notification - a message that is sent to the client at the initiative of the server.

Based on this meta-file EasyRpc-generator automatically creates classes for client and server.

Client and server interaction takes place in asynchronous mode using Boost-based TCP connections. Server-side notification of the client is supported.

Rpc example:
rpc (double value, std::string error_message) plus(double arg1, double arg2)

Client example:
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

Server example:
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