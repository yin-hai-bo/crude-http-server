#pragma once
#include <WinSock2.h>

class WinSockInitialize {
    bool initialized_;
public:
    WinSockInitialize()
        : initialized_(false)
    {}
    WinSockInitialize(const WinSockInitialize &) = delete;
    WinSockInitialize & operator = (const WinSockInitialize &) = delete;
    ~WinSockInitialize() {
        if (initialized_) {
            ::WSACleanup();
        }
    }

    bool Execute() {
        if (!this->initialized_) {
            WSADATA wsaData;
            this->initialized_ = (0 == ::WSAStartup(MAKEWORD(2, 0), &wsaData));
        }
        return this->initialized_;
    }
};

class Socket {
public:
    explicit Socket(SOCKET sock)
        : sock_(sock)
    {}
    Socket(const Socket &) = delete;
    Socket & operator = (const Socket &) = delete;
    Socket(Socket && other) noexcept
        : sock_(INVALID_SOCKET)
    {
        this->swap(other);
    }
    Socket & operator = (Socket && other) noexcept {
        this->swap(other);
        return *this;
    }
    ~Socket() noexcept {
        Close();
    }

    void swap(Socket & other) noexcept {
        if (this != &other) {
            auto const tmp = other.sock_;
            other.sock_ = this->sock_;
            this->sock_ = tmp;
        }
    }

    void Close() noexcept {
        if (IsValid()) {
            ::closesocket(sock_);
            sock_ = INVALID_SOCKET;
        }
    }

    bool IsValid() const {
        return INVALID_SOCKET != sock_;
    }

    int Shutdown(int how) const {
        return ::shutdown(sock_, how);
    }

    int Bind(const sockaddr_in & addr) const {
        return ::bind(sock_, (const sockaddr *)&addr, sizeof(sockaddr_in));
    }

    int Listen(int backlog) const {
        return ::listen(sock_, backlog);
    }

    Socket Accept(sockaddr_in * out_peer_addr) const {
        int len = sizeof(sockaddr_in);
        auto s = ::accept(sock_, (sockaddr *)out_peer_addr, &len);
        return Socket(s);
    }

    int Read(char * buf, size_t len) const {
        return ::recv(sock_, buf, (int)len, 0);
    }

    int Write(const void * data, size_t len) const {
        return ::send(sock_, (const char *)data, (int)len, 0);
    }

private:
    SOCKET sock_;
};