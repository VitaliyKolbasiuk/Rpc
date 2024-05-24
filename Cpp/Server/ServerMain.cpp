#include "TcpServer.h"
#include "RpcModel.h"

int main(int argc, char *argv[])
{
    std::thread serverThread(
        []
        {
            boost::asio::io_context io_context;

            RpcModel rpcModel;
            TcpServer server(io_context, rpcModel, 1234);
            server.execute();
        });
    serverThread.detach();
    int a;
    while (true)
    {
        a = 0;
    }
}
