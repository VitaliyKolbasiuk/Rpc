#include "ModernRpcTcpServer.h"
#include "RpcModel.h"

int main(int argc, char *argv[])
{
    std::thread serverThread(
        []
        {
            boost::asio::io_context io_context;

            RpcModel rpcModel;
            ModernRpcTcpServer server(io_context, rpcModel, 1234);
            server.run();
        });
    serverThread.detach();
    int a;
    while (true)
    {
        a = 0;
    }
}
