#pragma once
#include <memory>
#include <thread>
#include <vector>
#include "SockHelper.h"
#include "Session.h"

class HttpListener {
public:
    HttpListener();
    HttpListener(const HttpListener &) = delete;
    HttpListener & operator = (const HttpListener &) = delete;
    ~HttpListener();

    bool IsRunning() const { return sock_.IsValid(); }
    bool Start(const TCHAR * address, uint16_t port);
    void Close();

private:
    Socket sock_;
    volatile bool stopFlag_;
    std::unique_ptr<std::thread> workThread_;
    
    std::vector<std::unique_ptr<Session>> sessions_;

private:
    void doAccept();
};

