#pragma once
#include <thread>
#include "SockHelper.h"

class Session {
public:
    explicit Session(Socket && sock);
    Session(const Session &) = delete;
    Session & operator = (const Session &) = delete;
    ~Session();

    bool Start();
    void Close();
private:
    Socket sock_;
    std::unique_ptr<std::thread> thread_;

    volatile bool stopFlag_;
    void threadProc();
};
