#include "pch.h"
#include "RequestParser.h"

bool RequestParser::AppendRequestData(const char * data, size_t len) {
    request_.append(data, len);
    auto const endOfRequest = request_.find("\r\n\r\n");
    if (endOfRequest != std::string::npos) {
        request_.erase(endOfRequest);
        return true;
    }
    return false;
}

bool RequestParser::Parse(RequestParser::Result * result) const {
    result->httpMethod[0] = result->resPath[0] = '\0';

    size_t p = request_.find(' ');
    if (p == std::string::npos) {
        return false;
    }
    strncpy_s(result->httpMethod, request_.c_str(), p);

    ++p;
    size_t const p2 = request_.find(' ', p);
    if (p2 == std::string::npos) {
        return false;
    }
    strncpy_s(result->resPath, &request_.c_str()[p], p2 - p);

    return true;
}