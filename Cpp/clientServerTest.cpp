#include "Server/ModernRpcTcpServer.h"
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
            ModernRpcTcpServer server(io_context, rpcModel, 1234);
            server.run();
        });
    serverThread.detach();

    // CLIENT
    boost::asio::io_context ioContext;

    std::shared_ptr<RpcClient> rpcClient = std::make_shared<RpcClient>();
    auto tcpClient = std::make_shared<TcpClient>(ioContext, rpcClient);
    rpcClient->setTcpClient(tcpClient);

    std::promise<bool> promise;
    auto future = promise.get_future();
    tcpClient->connect("127.0.0.1", 1234, promise);

    std::thread clientThread([&ioContext]{
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard(ioContext.get_executor());
        ioContext.run();
        std::cout << "Context has stopped";
    });
    clientThread.detach();

    if (!future.get())
    {
        std::cerr << "No connection" << std::endl;
        return 0;
    }

    double result;
    if (std::string error = rpcClient->calculate(Operations::plus, 3, 5, result); !error.empty())
    {
        std::cerr << "Error while calculating" << std::endl;
    }

    assert (result == 8);

    std::cout << "Result = " << result << std::endl;

    int a;
    while (true)
    {
        a = 0;
    }
}
