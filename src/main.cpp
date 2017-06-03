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

void draw_fonts(HDC, std::vector<font::EnumFontInfo> &, size_t=0);

LRESULT CALLBACK MainFrameProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	static std::vector<font::EnumFontInfo> ff;
	static size_t ff_skipped = 0;

	switch(m)
	{
		default: break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(h, &ps);
			draw_fonts(ps.hdc, ff, ff_skipped);
			EndPaint(h, &ps);
			return 0;
		}

		case WM_CREATE:
		{
			HDC dc = GetDC(h);
			font::list_fonts(ff, ANSI_CHARSET, true, dc);
			ReleaseDC(h, dc);

			printf(" %d fonts found\n", ff.size());
			return 0;
		}

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

void draw_fonts(HDC dc, std::vector<font::EnumFontInfo> & ff, size_t skip)
{
	font::list_fonts(ff, ANSI_CHARSET, true, dc);

	unsigned const margin = 8;
	HWND const parent = WindowFromDC(dc);
	RECT cr;
	GetClientRect(parent, &cr);
	unsigned y = 0;

	for (size_t i=skip; i<ff.size(); ++i)
	{
		if (y + ff[i].elfe.elfLogFont.lfHeight > cr.bottom - margin)
			break;

		// printf(" %3d | %s | %s | %s | %d | %s\n", i+1, ft,
		// 	ff[i].elfe.elfLogFont.lfFaceName,
		// 	ff[i].elfe.elfStyle,
		// 	ff[i].elfe.elfLogFont.lfCharSet,
		// 	ff[i].elfe.elfScript);

		char const * text = (char const *) ff[i].elfe.elfFullName;

		HFONT hf = CreateFontIndirect(&ff[i].elfe.elfLogFont);
		HGDIOBJ old = SelectObject(dc, hf);
		TextOut(dc, margin, margin + y, text, strlen(text));
		SelectObject(dc, old);
		DeleteObject(hf);

		y += ff[i].elfe.elfLogFont.lfHeight;
	}
}
