#pragma once
#include <string>

class RequestParser {
public:

    bool AppendRequestData(const char * data, size_t len);

    struct Result {
        char httpMethod[16];
        char resPath[512];
    };

    bool Parse(Result * result) const;

private:
    std::string request_;
};
