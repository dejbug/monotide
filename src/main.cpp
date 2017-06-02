#include <windows.h>
#include <stdio.h>
#include "lib_window.h"

using namespace lib;

LRESULT CALLBACK MainFrameProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE i, HINSTANCE, LPSTR, int)
{
	char const * const class_name = "MAINFRAME";

	window::create_class(class_name, MainFrameProc, i);
	HWND frame = window::create_frame(class_name, i);
	SetWindowText(frame, "anika-redux");
	window::set_size(frame, 1040, 800);
	window::center_to_parent(frame);
	UpdateWindow(frame);
	ShowWindow(frame, SW_SHOW);

	MSG msg;
	window::run_main_loop(msg);
	return msg.wParam;
}

LRESULT CALLBACK MainFrameProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	switch(m)
	{
		default: break;

		case WM_KEYDOWN:
		{
			switch(w)
			{
				case VK_ESCAPE:
					SendMessage(h, WM_CLOSE, 0, 0);
					break;

				case VK_F5:
					InvalidateRect(h, NULL, TRUE);
					UpdateWindow(h);
					break;
			}
			return 0;
		}

		case WM_CLOSE:
			DestroyWindow(h);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(h, m, w, l);
}
