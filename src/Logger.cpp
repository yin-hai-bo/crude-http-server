#include "pch.h"
#include "Logger.h"

static HWND editCtrl;

void Logger::Init(HWND theEditCtrl) {
    editCtrl = theEditCtrl;
    ::SendMessage(editCtrl, EM_SETREADONLY, TRUE, 0);
    ::SendMessage(editCtrl, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

void Logger::SetPosition(int x, int y, int width, int height) {
    ::SetWindowPos(editCtrl, NULL, x, y, width, height, SWP_SHOWWINDOW);
}

static void makeCurrentTimeString(TCHAR * buf, size_t len) {
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    _sntprintf_s(buf, len, _TRUNCATE,
        _T("%04u-%02u-%02u %02u:%02u:%02u "),
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond);
}

static void append(const TCHAR * text) {
    auto const len = ::GetWindowTextLength(editCtrl);
    ::SendMessage(editCtrl, EM_SETSEL, len, len);
    ::SendMessage(editCtrl, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text));
}

void Logger::Append(const TCHAR * text, bool changeLine) {
    TCHAR timeStr[128];
    makeCurrentTimeString(timeStr, sizeof(timeStr) / sizeof(timeStr[0]));
    append(timeStr);

    append(text);

    if (changeLine) {
        append(_T("\r\n"));
    }
}

void Logger::AppendFormat(bool changeLine, const TCHAR * fmt, ...) {
    va_list args;
    va_start(args, fmt);

    TCHAR buffer[4096];
    _vsntprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), _TRUNCATE, fmt, args);
    Append(buffer, changeLine);

    va_end(args);
}