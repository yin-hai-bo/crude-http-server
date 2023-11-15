#include "pch.h"
#include "resource.h"
#include "TrayIcon.h"
#include "Menu.h"
#include "Logger.h"
#include "HttpListener.h"
#include <memory>

#define MY_WM_TRAYICON (WM_USER + 1)

class AboutBox {
public:
	static INT_PTR CALLBACK DlgCallback(HWND const dlg, UINT const message, WPARAM const wParam, LPARAM) {
		switch (message) {
		case WM_INITDIALOG:
			adjust_position(dlg);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
				EndDialog(dlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
		}
		return (INT_PTR)FALSE;
	}
private:
	static void adjust_position(HWND const dlg) {
		RECT rcParent;
		if (!GetWindowRect(GetParent(dlg), &rcParent)) {
			if (!GetWindowRect(GetDesktopWindow(), &rcParent)) {
				return;
			}
		}

		RECT rcDialog;
		GetWindowRect(dlg, &rcDialog);

		auto const dlgWidth = rcDialog.right - rcDialog.left;
		auto const dlgHeight = rcDialog.bottom - rcDialog.top;

		auto const xPos = (rcParent.left + rcParent.right - dlgWidth) / 2;
		auto const yPos = (rcParent.top + rcParent.bottom - dlgHeight) / 2;

		SetWindowPos(dlg, NULL, xPos, yPos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
};

/**
 * On WM_COMMAND message
 *
 * @return True if the message has been processed, otherwise return false.
*/
static bool onCommandMessage(HWND const wnd, const int wmId) {
	switch (wmId) {
	case IDM_ABOUT:
		DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUTBOX), wnd, AboutBox::DlgCallback);
		break;
	case IDM_EXIT:
		DestroyWindow(wnd);
		break;
	default:
		return false;
	}
	return true;
}

static void onCreateMessage(HWND const wnd) {
	auto h = ::CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""),
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, 300, 200,
		wnd, NULL, NULL, nullptr);
	Logger::Init(h);
}

static void onSizeMessage(HWND const wnd) {
	RECT rect;
	::GetClientRect(wnd, &rect);
	Logger::SetPosition(0, 0, rect.right - rect.left, rect.bottom - rect.top);
}

static void onPaintMessage(HWND const wnd) {
	PAINTSTRUCT ps;
	HDC const hdc = BeginPaint(wnd, &ps);

	// TODO: .........

	EndPaint(wnd, &ps);
}

static void popupContextMenu(HWND wnd) {
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	Menu menu((HINSTANCE)GetWindowLongPtr(wnd, GWLP_HINSTANCE), (LPCTSTR)IDC_TRAYICON_MENU);
	menu.TrackPopup(wnd, 0, cursorPos.x, cursorPos.y);
}

static void onMyTrayIcon(HWND wnd, LPARAM lParam) {
	switch (lParam) {
	case WM_LBUTTONDOWN:
		ShowWindow(wnd, SW_SHOWNORMAL);
		break;
	case WM_RBUTTONUP:
		popupContextMenu(wnd);
		break;
	}
}

static LRESULT wndProc(HWND const wnd, UINT const message, WPARAM const wParam, LPARAM const lParam) {
	switch (message) {
	case WM_CREATE:
		onCreateMessage(wnd);
		break;
	case WM_SIZE:
		onSizeMessage(wnd);
		break;
	case WM_COMMAND:
		if (!onCommandMessage(wnd, LOWORD(wParam))) {
			return DefWindowProc(wnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		onPaintMessage(wnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		ShowWindow(wnd, SW_HIDE);
		break;
	case MY_WM_TRAYICON:
		onMyTrayIcon(wnd, lParam);
		break;
	default:
		return DefWindowProc(wnd, message, wParam, lParam);
	}
	return 0;
}

static ATOM registerClass(HINSTANCE const inst, const TCHAR * const wndClass) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = inst;
	wcex.hIcon = wcex.hIconSm = LoadIcon(inst, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = (LPCTSTR)IDC_MAIN_MENU;
	wcex.lpszClassName = wndClass;
	return RegisterClassEx(&wcex);
}

static HWND createMainWindow(HINSTANCE const inst, TCHAR const * const className, int const showCmd) {
	TCHAR title[512];
	LoadString(inst, IDS_APP_TITLE, title, sizeof(title) / sizeof(title[0]));

	HWND const wnd = CreateWindow(
		className, title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		nullptr, nullptr, inst, nullptr);
	if (!wnd) {
		return NULL;
	}
	ShowWindow(wnd, showCmd);
	UpdateWindow(wnd);
	return wnd;
}

int APIENTRY _tWinMain(
	_In_ HINSTANCE inst,
	_In_opt_ HINSTANCE,
	_In_ LPTSTR,
	_In_ int showCmd) {

	static const TCHAR CLASS_NAME[] = _T("{7222A6D0-B6C7-4796-8C72-558718A50255}");
	ATOM const registeredClass = registerClass(inst, CLASS_NAME);
	if (0 == registeredClass) {
		return 0;
	}

	HWND const wnd = createMainWindow(inst, (TCHAR const *)registeredClass, showCmd);
	if (!wnd) {
		return 0;
	}

	WinSockInitialize winSockInitialize;
	if (!winSockInitialize.Execute()) {
		// TODO: show error dialog box then exit.
		PostQuitMessage(-1);
	}

	int resultCode;
	do {
		TrayIcon trayIcon(wnd, MY_WM_TRAYICON);
		HttpListener httpListener;
		httpListener.Start("127.0.0.1", 7301);

		HACCEL const accelTable = LoadAccelerators(inst, MAKEINTRESOURCE(IDC_ACCEL));
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0)) {
			if (!TranslateAccelerator(msg.hwnd, accelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		resultCode = static_cast<int>(msg.wParam);
	} while (false);

	return resultCode;
}
