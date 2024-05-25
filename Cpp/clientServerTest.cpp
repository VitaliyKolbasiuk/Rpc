#include "Server/TcpServer.h"
#include "Server/RpcModel.h"

#include "Client/TcpClient.h"
#include "Client/RpcClient.h"

int main(int argc, char *argv[])
{
    // SERVER
    std::thread serverThread(
        []
        {
            boost::asio::io_context io_context;

            RpcModel rpcModel;
            TcpServer server(io_context, rpcModel, 1234);
            server.execute();
        });
    serverThread.detach();

    // CLIENT
    boost::asio::io_context ioContext;

    std::shared_ptr<RpcClient> rpcClient = std::make_shared<RpcClient>();
    auto tcpClient = std::make_shared<TcpClient>(ioContext, rpcClient);
    rpcClient->setTcpClient(tcpClient);
    tcpClient->connect("127.0.0.1", 1234);

    std::thread clientThread([&ioContext]{
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard(ioContext.get_executor());
        ioContext.run();
        std::cout << "Context has stopped";
    });
    clientThread.detach();

    int a;
    while (true)
    {
        a = 0;
    }
}
