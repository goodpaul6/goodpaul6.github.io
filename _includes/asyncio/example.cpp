// example.cpp
// An echo server demonstrating the API we'll be building.

#include <cstring>
#include <iostream>
#include <list>
#include <memory>

#include "helpers.hpp"
#include "io_context.hpp"
#include "socket.hpp"

struct Server;

struct Client {
    IOContext* io_context = nullptr;

    Socket socket;
    bool closed = false;

    char buf[128];

    Client(IOContext& io_context, Socket socket)
        : io_context{&io_context}, socket{std::move(socket)} {
        // Start the receive loop
        io_context.async_recv(this->socket, buf, sizeof(buf),
                              [this](int len) { recv_handler(len); });
    }

    void recv_handler(int len) {
        if (len <= 0) {
            closed = true;
            return;
        }

        std::printf("Received: %.*s.\n", len, buf);

        // Copy the buffer onto the heap and tie its lifetime to
        // that of the lambda
        auto shared_buf = std::make_shared<std::vector<char>>();

        shared_buf->resize(len);
        std::memcpy(shared_buf->data(), buf, len);

        auto buf_data = shared_buf->data();

        // Write everything we received back
        async_send_all(*io_context, socket, buf_data, len,
                       [shared_buf = std::move(shared_buf)](int) {});

        // Queue this client to receive more
        io_context->async_recv(this->socket, buf, sizeof(buf),
                               [this](int len) { recv_handler(len); });
    }
};

struct Server {
    IOContext io_context;

    Socket socket;

    std::list<Client> clients;

    Server(unsigned short port) : socket{Socket::ListenParams{port}} {}

    void run() {
        io_context.async_accept(
            socket, [this](Socket client_socket) { accept_handler(std::move(client_socket)); });
        io_context.run();
    }

    void accept_handler(Socket client_socket) {
        // Remove disconnected clients
        auto clients_end =
            std::remove_if(clients.begin(), clients.end(), [](auto& c) { return c.closed; });

        clients.erase(clients_end, clients.end());

        // Add the new socket
        clients.emplace_back(io_context, std::move(client_socket));

        // Continue accepting more clients
        io_context.async_accept(
            socket, [this](Socket client_socket) { accept_handler(std::move(client_socket)); });
    }
};

int main(int argc, char** argv) {
#ifdef SERVER
    // Async IO server
    Server server{42690};

    server.run();
#else
    // Simple synchronous client
    Socket socket{Socket::ConnectParams{argv[1], static_cast<unsigned short>(std::atoi(argv[2]))}};

    socket.set_non_blocking(false);

    std::string s;

    while (std::getline(std::cin, s)) {
        socket.send(s.data(), s.size());
        socket.recv(s.data(), s.size());

        std::printf("Received: %s\n", s.c_str());
    }
#endif

    return 0;
}
