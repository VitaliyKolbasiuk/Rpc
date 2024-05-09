#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 8080));

        std::string message_body = "Hello from C++ Boost.Asio!";
        uint32_t message_length = message_body.size();

        // Send the message length followed by the message body
        boost::asio::write(socket, boost::asio::buffer(&message_length, sizeof(message_length)));
        boost::asio::write(socket, boost::asio::buffer(message_body));

        // Read the response length
        uint32_t response_length;
        boost::asio::read(socket, boost::asio::buffer(&response_length, sizeof(response_length)));

        // Read the response body
        std::vector<char> response_body(response_length);
        boost::asio::read(socket, boost::asio::buffer(response_body));

        std::cout << "Reply from server: " << std::string(response_body.begin(), response_body.end()) << std::endl;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
