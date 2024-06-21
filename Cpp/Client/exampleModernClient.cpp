#include "ModernRpcClientBase.h"
#include "RpcClient.h"

int main()
{
    boost::asio::io_context context;
    RpcClient rpcClient(context);

    std::promise<bool> promise;
    rpcClient.connect("127.0.0.1", 12345, promise);
    std::thread contextThread([&context]{
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> workGuard(context.get_executor());
        context.run();
        std::cout << "Context has stopped" << std::endl;
    });
    contextThread.detach();

    if (!promise.get_future().get())
    {
        std::cerr << "Connection was not established" << std::endl;
        return -1;
    }

    std::mutex mtx;
    std::condition_variable cv;
    int operations = 3;

    auto callback = [&operations, &mtx, &cv](double value, std::string error_message)
    {
        std::cout << "Result: ";
        if (!error_message.empty())
        {
            std::cout << error_message << std::endl;
        }
        else
        {
            std::cout << value << std::endl;
        }
        if (--operations == 0)
        {
            cv.notify_one();
        }
    };

    rpcClient.plus(1, 2, callback);
    rpcClient.minus(8, 4, callback);
    rpcClient.plus(5, 6, callback);

    // Wait for all operations to complete
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&operations]{ return operations == 0; });

    // Stop the context to allow the context thread to finish
    context.stop();
}