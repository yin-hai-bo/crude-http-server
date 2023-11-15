#include "pch.h"
#include "Session.h"
#include "RequestParser.h"

using thread = std::thread;

Session::Session(Socket && sock)
    : sock_(std::move(sock))
    , stopFlag_(false)
{}

Session::~Session() {
    Close();
}

void Session::Close() {
    stopFlag_ = true;
    sock_.Close();
    if (thread_) {
        thread_->join();
        thread_.reset();
    }
}

bool Session::Start() {
    if (!sock_.IsValid()) {
        return false;
    }
    if (thread_) {
        return false;
    }
    thread_.reset(new thread(&Session::threadProc, this));
    return true;
}

static void writeResponse(const Socket & sock, int statusCode, const void * data = nullptr, size_t len = 0) {
    std::ostringstream oss;
    oss << "HTTP/1.0 " << statusCode << "\r\n";
    do {
        if (statusCode != 200) {
            oss << "\r\n";
            break;
        }
    
        if (data && len) {
            oss << "Content-Type: text/html; charset = utf-8\r\n";
            oss << "Content-Length: " << len << "\r\n\r\n";
            oss.write((const char *)data, len);
        } else {
            oss << "\r\n";
        }
    } while (false);

    auto const s = oss.str();
    sock.Write(s.c_str(), s.size());
    OutputDebugStringA(s.c_str());
}

static void doResponse(const Socket & sock, const RequestParser::Result & request) {
    if (_stricmp("GET", request.httpMethod) != 0) {
        writeResponse(sock, 400);
        return;
    }
    // FIXME: read file and response.
    writeResponse(sock, 200, request.resPath, strlen(request.resPath));
}

void Session::threadProc() {
    RequestParser parser;
    char buffer[2048];
    while (!stopFlag_) {
        int const bytes = sock_.Read(buffer, sizeof(buffer) - 1);
        if (stopFlag_) {
            break;
        }
        if (bytes <= 0) {
            // TODO: write error log.
            break;
        }

        if (parser.AppendRequestData(buffer, bytes)) {
            RequestParser::Result result;
            if (parser.Parse(&result)) {
                doResponse(sock_, result);
            }
            break;
        }
    }
}