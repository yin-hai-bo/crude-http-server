#include "pch.h"
#include "HttpListener.h"

HttpListener::HttpListener()
    : sock_(INVALID_SOCKET)
    , stopFlag_(false)
{}

HttpListener::~HttpListener() {
    Close();
}

void HttpListener::Close() {
    stopFlag_ = true;
    if (sock_.IsValid()) {
        sock_.Shutdown(SD_BOTH);
        sock_.Close();
        if (workThread_) {
            workThread_->join();
            workThread_.reset();
        }
    }
}

bool HttpListener::Start(const char * address, uint16_t port) {
    assert(!sock_.IsValid());
    assert(!workThread_);

    sockaddr_in addr;
    if (1 != ::inet_pton(AF_INET, address, &addr.sin_addr)) {
        ::WSASetLastError(WSA_INVALID_PARAMETER);
        return false;
    }

    if (sock_.IsValid()) {
        ::WSASetLastError(WSAEALREADY);
        return false;
    }

    sock_ = Socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (!sock_.IsValid()) {
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (0 == sock_.Bind(addr) && 0 == sock_.Listen(5)) {
        this->sessions_.reserve(8);
        this->stopFlag_ = false;
        this->workThread_ = std::make_unique<std::thread>(&HttpListener::doAccept, this);
        return true;
    } else {
        sock_.Close();
        return false;
    }
}

void HttpListener::doAccept() {
    while (!stopFlag_) {
        sockaddr_in peerAddr;
        Socket client = sock_.Accept(&peerAddr);
        if (stopFlag_) {
            break;
        }
        auto session = std::make_unique<Session>(std::move(client));
        session->Start();
        sessions_.push_back(std::move(session));
    }

    for (auto & s : sessions_) {
        s->Close();
    }
    sessions_.clear();
}