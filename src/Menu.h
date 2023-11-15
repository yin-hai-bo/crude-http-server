#pragma once
#include <Windows.h>
#include <tchar.h>

class Menu {
	HMENU const menu_;
public:
	Menu(HINSTANCE inst, LPCTSTR name)
		: menu_(LoadMenu(inst, name))
	{}
	Menu(const Menu &) = delete;
	Menu & operator = (const Menu &) = delete;
	~Menu() {
		if (menu_) {
			DestroyMenu(menu_);
		}
	}

	operator bool() const {
		return menu_;
	}
	bool operator !() const {
		return !menu_;
	}

	void TrackPopup(HWND wnd, int subMenuPos, int x, int y) {
		auto const popup = GetSubMenu(menu_, subMenuPos);
		::TrackPopupMenu(popup, TPM_RIGHTALIGN | TPM_TOPALIGN,
			x, y, 0, wnd, nullptr);
	}
};
