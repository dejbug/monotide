#include <windows.h>
#include <stdio.h>
#include <vector>
#include "macros.h"
#include "snippets.h"
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
	static UINT rows_per_scroll = 1;

	switch(m)
	{
		default: break;

		case WM_MOUSEWHEEL:
		{
			short const zDelta = (short) HIWORD(w);

			if (zDelta > 0 && ff_skipped >= rows_per_scroll)
			{
				ff_skipped -= rows_per_scroll;
				InvalidateRect(h, NULL, TRUE);
			}
			if (zDelta < 0 && ff_skipped + rows_per_scroll < ff.size())
			{
				ff_skipped += rows_per_scroll;
				InvalidateRect(h, NULL, TRUE);
			}

			UpdateWindow(h);
		}

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
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES,
				0, &rows_per_scroll, 0);

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
	HBRUSH const frame_brush = (HBRUSH) GetStockObject(DC_BRUSH);
	SIZE const frame_extra = {3, 2};
	SIZE const padding = {8, 8};
	SIZE client_size;
	lib::window::get_inner_size(WindowFromDC(dc), client_size);
	int const bottom_y = client_size.cy - padding.cy;

	int y = 0;
	// INT * char_widths = new INT[256];
	// INT * text_offsets = new INT[1024];

	for (size_t i=skip; i<ff.size(); ++i)
	{
		char const * text = (char const *) ff[i].elfe.elfFullName;
		size_t const text_len = strlen(text);

		RECT tr = {padding.cx, padding.cy + y, 0, 0};
		snippets::calc_text_rect_1(tr, dc, ff[i], text, text_len);

		int const next_y = tr.bottom - padding.cy + frame_extra.cy + 1;

		if (next_y > bottom_y) break;

		InflateRect(&tr, frame_extra.cx, frame_extra.cy);
		SetDCBrushColor(dc, RGB(100,100,100));
		FrameRect(dc, &tr, frame_brush);

		RECT rc = {padding.cx, padding.cy + y, 0, 0};
		lib::font::draw_font_label(dc, rc, ff[i]);

		// font::print_font_info(ff[i]);

		y = next_y;
	}

	// delete[] text_offsets;
	// delete[] char_widths;
}
