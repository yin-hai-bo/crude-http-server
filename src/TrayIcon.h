#pragma once
#include <Windows.h>
#include <shellapi.h>

class TrayIcon {
public:
	TrayIcon(HWND wnd, UINT msgId);
	TrayIcon(const TrayIcon&) = delete;
	TrayIcon& operator = (const TrayIcon&) = delete;
	~TrayIcon();

private:
	NOTIFYICONDATA nid_;
};

