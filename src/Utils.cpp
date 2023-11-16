#include "pch.h"
#include "Utils.h"
#include <vector>

std::string Utils::WideToAnsi(const wchar_t * ws) {
    size_t const chars = wcslen(ws);
    if (chars == 0) {
        return std::string();
    }

    std::vector<char> buf(chars * 2 + 1);
    size_t convertedSize;
    wcstombs_s(&convertedSize, &buf[0], buf.size(), ws, _TRUNCATE);
    return std::string(&buf[0]);
}
