#include "pch.h"
#include "resource.h"
#include "TrayIcon.h"

TrayIcon::TrayIcon(HWND wnd, UINT msgId) {

    HINSTANCE const inst = (HINSTANCE)GetWindowLongPtr(wnd, GWLP_HINSTANCE);

    nid_.cbSize = sizeof(NOTIFYICONDATA);
    nid_.hWnd = wnd;
    nid_.uID = 1;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = msgId;
    nid_.hIcon = LoadIcon(inst, IDI_APPLICATION);
    LoadString(inst, IDS_APP_TITLE, nid_.szTip, sizeof(nid_.szTip) / sizeof(nid_.szTip[0]));
    Shell_NotifyIcon(NIM_ADD, &nid_);
}

TrayIcon::~TrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid_);
}

// ��������Ϣ������
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_USER + 1:
        // ��������ͼ�����Ϣ
        switch (lParam) {
        case WM_LBUTTONUP:
            MessageBox(NULL, L"Tray Icon Clicked!", L"Tray Icon", MB_ICONINFORMATION);
            break;
        case WM_RBUTTONUP:
            // �����Ҽ��˵�
            MessageBox(NULL, L"Right-clicked on Tray Icon!", L"Tray Icon", MB_ICONINFORMATION);
            break;
        }
        break;
    case WM_DESTROY:
        // �˳�����
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
