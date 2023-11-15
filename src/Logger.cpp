#include "pch.h"
#include "Logger.h"

static HWND editCtrl;

void Logger::Init(HWND theEditCtrl) {
    editCtrl = theEditCtrl;
    ::SendMessage(editCtrl, EM_SETREADONLY, TRUE, 0);
}

void Logger::SetPosition(int x, int y, int width, int height) {
    ::SetWindowPos(editCtrl, NULL, x, y, width, height, SWP_SHOWWINDOW);
}

static void append(const TCHAR * text) {
    auto const len = ::GetWindowTextLength(editCtrl);
    ::SendMessage(editCtrl, EM_SETSEL, len, len);
    ::SendMessage(editCtrl, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(text));
}

void Logger::Append(const TCHAR * text, bool changeLine) {
    append(text);
    if (changeLine) {
        append(_T("\r\n"));
    }
}

