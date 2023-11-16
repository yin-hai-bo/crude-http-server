#include "pch.h"
#include "HttpListener.h"
#include "Utils.h"
#include "Logger.h"

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

bool HttpListener::Start(const TCHAR * const address, uint16_t const port) {
    assert(!sock_.IsValid());
    assert(!workThread_);

    Logger::AppendFormat(true, _T("Try to listen %s:%u ..."), address, port);

#ifdef UNICODE
    const std::string addressAnsi = Utils::WideToAnsi(address);
    const char * addressStr = addressAnsi.c_str();
#else
    const char * addressStr = address;
#endif

    sockaddr_in addr;
    if (1 != ::inet_pton(AF_INET, addressStr, &addr.sin_addr)) {
        Logger::AppendFormat(true, _T("Invalid address: %s"), address);
        return false;
    }

    if (sock_.IsValid()) {
        Logger::Append(_T("Listening is already in progress. Cannot attempt to start listening again."));
        return false;
    }

    sock_ = Socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (!sock_.IsValid()) {
        Logger::AppendFormat(true, _T("Create socket failed, error #%d"), ::WSAGetLastError());
        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    do {
        if (0 != sock_.Bind(addr)) {
            Logger::AppendFormat(true, _T("Bind socket to listen address failed, error #%d"), ::WSAGetLastError());
            break;
        }

        if (0 != sock_.Listen(5)) {
            Logger::AppendFormat(true, _T("Try to listen failed, error #%d"), ::WSAGetLastError());
            break;
        }

        this->sessions_.reserve(8);
        this->stopFlag_ = false;
        this->workThread_ = std::make_unique<std::thread>(&HttpListener::doAccept, this);
        Logger::Append(_T("Service has been successfully started."));
        return true;
    } while (false);
        
    sock_.Close();
    return false;
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