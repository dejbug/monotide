#include <windows.h>
#include <stdio.h>
#include <vector>
#include "lib_window.h"
#include "lib_font.h"

using namespace lib;

LRESULT CALLBACK MainFrameProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE i, HINSTANCE, LPSTR, int)
{
	char const * const class_name = "MAINFRAME";

	std::vector<font::EnumFontInfo> ff;
	// font::list_fonts(ff, ANSI_CHARSET, true);
	font::list_fonts(ff, DEFAULT_CHARSET);

	printf("\n%d fonts found\n", ff.size());

	for(size_t i=0; i<ff.size(); ++i)
	{
		char const * ft = "?";
		switch(ff[i].FontType)
		{
			case DEVICE_FONTTYPE: ft = "dev"; break;
			case RASTER_FONTTYPE: ft = "ras"; break;
			case TRUETYPE_FONTTYPE: ft = "ttf"; break;
		};

		printf(" %3d | %s | %s | %s | %d | %s\n", i+1, ft,
			ff[i].elfe.elfLogFont.lfFaceName,
			ff[i].elfe.elfStyle,
			ff[i].elfe.elfLogFont.lfCharSet,
			ff[i].elfe.elfScript);
	}

	return 0;

	window::create_class(class_name, MainFrameProc, i);
	HWND frame = window::create_frame(class_name, i);
	SetWindowText(frame, "monotide-redux");
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
