#pragma once
#include <Windows.h>
#include <tchar.h>

class Logger {
public:
    static void Init(HWND editCtrl);
    static void SetPosition(int x, int y, int width, int height);
    static void Append(const TCHAR * msg, bool changeLine = true);
};

