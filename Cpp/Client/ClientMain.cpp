#include "TcpClient.h"
#include "RpcClient.h"

#include <memory>
#include <boost/asio.hpp>

int main()
{
    boost::asio::io_context ioContext;

    std::shared_ptr<RpcClient> rpcClient = std::make_shared<RpcClient>();
    auto tcpClient = std::make_shared<TcpClient>(ioContext, rpcClient);
    rpcClient->setTcpClient(tcpClient);
    tcpClient->connect("127.0.0.1", 1234);

    std::thread ([&ioContext]{
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard(ioContext.get_executor());
        ioContext.run();
        std::cout << "Context has stopped" << std::endl;
    }).detach();

    int a;
    while (true)
    {
        a = 1;
    }
}