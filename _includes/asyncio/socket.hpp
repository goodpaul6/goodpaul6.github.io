#pragma once

#include <optional>

struct Socket {
    // These structs make it easy to distinguish between server
    // and client sockets without having to write separate
    // functions or to start the Socket off in some uninitialized
    // state.
    struct ListenParams { 
        unsigned short port = 6969;
        int backlog = 4;
    };

    struct ConnectParams {
        const char* host = nullptr;
        unsigned short port = 6969;
    };

    explicit Socket(int fd);
    explicit Socket(const ListenParams& params);
    explicit Socket(const ConnectParams& params);

    Socket(Socket&& other);
    Socket& operator=(Socket&& other);

    // We don't allow copying the Socket for obvious reasons.
    Socket(const Socket& other) = delete;
    Socket& operator=(const Socket& other) = delete;

    ~Socket();

    // We use optional to return nullopt when the operation would block,
    // not to signal an error. That would throw an exception.
    std::optional<Socket> accept();

    int recv(char* buf, int maxlen);
    int send(const char* buf, int maxlen);

    int fd() const;

    void set_non_blocking(bool enabled);
    void set_no_delay(bool enabled);

private:
    int m_fd = -1;
};
