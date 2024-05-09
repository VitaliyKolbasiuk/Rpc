#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;

        // Create and bind the acceptor to the desired address and port
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        std::cout << "Server running on 127.0.0.1:8080" << std::endl;

        while (true) {
            // Accept incoming connections
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            // Read the length of the incoming message
            uint32_t message_length;
            boost::asio::read(socket, boost::asio::buffer(&message_length, sizeof(message_length)));

            // Read the body of the message
            std::string message_body(message_length, '\0');
            boost::asio::read(socket, boost::asio::buffer(&message_body[0], message_length));

            // Echo the message back to the client
            boost::asio::write(socket, boost::asio::buffer(&message_length, sizeof(message_length)));
            boost::asio::write(socket, boost::asio::buffer(message_body));
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
