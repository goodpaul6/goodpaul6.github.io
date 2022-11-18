// example.cpp
// An echo server demonstrating the API we'll be building.

#include "context.hpp"
#include "helpers.hpp"
#include "socket.hpp"

struct Server;

struct Client {
    Server* server = nullptr;

    Socket socket;
    bool closed = false;

    char buf[128];

    Client(Server& server, Socket socket) : server{&server}, socket{std::move(socket)} {
        // Start the receive loop
        server->io_context.async_recv(this->socket, buf, sizeof(buf),
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

        shared_buf.resize(len);
        std::memcpy(shared_buf->data(), buf, len);

        auto buf_data = shared_buf->data();

        // Write everything we received back
        async_send_all(server->io_context, socket, buf_data, len,
                       [shared_buf = std::move(shared_buf)](int) {});

        // Queue this client to receive more
        server->io_context.async_recv(this->socket, buf, sizeof(buf),
                                      [this](int len) { recv_handler(len); });
    }
};

struct Server {
    IOContext io_context;

    Socket socket;

    std::list<Client> clients;

    Server(unsigned short port) : socket{Socket::ListenParams{port}} {}

    void run() {
        io_context.async_accept(socket,
                                [this](Socket socket) { accept_handler(std::move(socket)); });
        io_context.run();
    }

    void accept_handler(Socket socket) {
        // Remove disconnected clients
        auto clients_end =
            std::remove_if(clients.begin(), clients.end(), [](auto& c) { return c.closed; });

        clients.erase(clients_end, clients.end());

        // Add the new socket
        clients.emplace_back(*this, std::move(socket));

        // Continue accepting more clients
        io_context.async_accept(socket,
                                [this](Socket socket) { accept_handler(std::move(socket)); });
    }
};

int main(int argc, char** argv) {
#ifdef SERVER
    // Async IO server
    Server server{42690};

    server.run();
#else
    // Simple synchronous client
    Socket socket{Socket::ConnectParams{argv[1], static_cast<unsigned short>(argv[2])}};

    socket.set_non_blocking(false);

    std::string s;

    while (std::getline(std::cin, s)) {
        send_all(socket, s.data(), s.size());
        recv_all(socket, s.data(), s.size());

        std::printf("Received: %s\n", s.c_str());
    }
#endif

    return 0;
}
