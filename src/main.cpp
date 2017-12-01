#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "lib_window.h"
#include "fontlist.h"

using namespace lib;

HWND hFontList = nullptr;

bool wm_create(HWND h, LPCREATESTRUCT)
{
	hFontList = window::create_child(WC_FONTLIST, h, IDC_FONTLIST, WS_BORDER);
	if (!hFontList) return false;

	window::set_size(hFontList, 500, 600);
	window::set_pos(hFontList, 10, 10);

	ShowWindow(hFontList, SW_SHOW);
	UpdateWindow(hFontList);

	SetFocus(hFontList);
	return true;
}

void wm_destroy(HWND)
{
	PostQuitMessage(0);
}

void wm_size(HWND h, UINT /*fwSizeType*/ , int /*cx*/, int /*cy*/)
{
	InvalidateRect(h, NULL, TRUE);
}

void wm_keydown(HWND h, UINT key, BOOL, int /*repeatCount*/, UINT /*flags*/)
{
	switch(key)
	{
		case VK_ESCAPE:
			window::close_window(h);
			break;

		case VK_F5:
			InvalidateRect(h, NULL, TRUE);
			UpdateWindow(h);
			break;
	}
}

void wm_command(HWND h, int id, HWND /*ctrl*/, UINT code)
{
	UINT const is_accelerator = (1 == code);

	if (is_accelerator) switch (id)
	{
		case IDM_ESCAPE:
			window::close_window(h);
			break;

		case IDM_F8:
			_tprintf(_T("hello accelerator!\n"));
			break;
	}
}

void wm_setfocus(HWND, HWND /*hwndLoseFocus*/)
{
	SetFocus(hFontList);
}

LRESULT CALLBACK MainFrameProc(HWND h, UINT m, WPARAM wParam, LPARAM lParam)
{
	switch(m)
	{
		default: return DefWindowProc(h, m, wParam, lParam);

		case WM_CLOSE: DestroyWindow(h); return 0;
		// case WM_ERASEBKGND: return 0;

		HANDLE_MSG(h, WM_CREATE, wm_create);
		HANDLE_MSG(h, WM_DESTROY, wm_destroy);
		HANDLE_MSG(h, WM_SIZE, wm_size);
		HANDLE_MSG(h, WM_KEYDOWN, wm_keydown);
		HANDLE_MSG(h, WM_COMMAND, wm_command);
		HANDLE_MSG(h, WM_SETFOCUS, wm_setfocus);
	}
}

// int WINAPI _tWinMain(HINSTANCE i, HINSTANCE, LPTSTR, int iCmdShow)
int WINAPI WinMain(HINSTANCE i, HINSTANCE, LPSTR, int iCmdShow)
{
	// InitCommonControls();
	RegisterFontList();

	LPCTSTR frame_cn = _T("monotide-MAINFRAME");
	window::create_class(frame_cn, MainFrameProc, i);

	HWND frame = window::create_frame(i, frame_cn);
	SetWindowText(frame, _T("monotide-redux 2"));
	window::set_size(frame, 600, 800);
	window::center_to_parent(frame);

	ShowWindow(frame, iCmdShow);
	UpdateWindow(frame);

	MSG msg;
	HACCEL haccel = LoadAccelerators(i, _T("accel_main"));
	window::run_main_loop(msg, frame, haccel);
	return msg.wParam;
}
