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

void draw_fonts(HWND, HDC, std::vector<font::EnumFontInfo> &, size_t=0);
void draw_info(HDC);

LRESULT CALLBACK MainFrameProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	static std::vector<font::EnumFontInfo> ff;
	static size_t ff_skipped = 0;
	static UINT rows_per_scroll = 1;
	static window::BackgroundDC offscreen;

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
				// UpdateWindow(h);
			}

			else if (zDelta < 0 && ff_skipped + rows_per_scroll < ff.size())
			{
				ff_skipped += rows_per_scroll;
				InvalidateRect(h, NULL, TRUE);
				// UpdateWindow(h);
			}

			return 0;
		}

		case WM_ERASEBKGND:
			offscreen.clear(0);
			draw_info(offscreen.handle);
			offscreen.flip();
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(h, &ps);
			EndPaint(h, &ps);

			draw_fonts(h, offscreen.handle, ff, ff_skipped);
			offscreen.flip();

			return 0;
		}

		case WM_SIZE:
		{
			HDC dc = GetDC(h);
			offscreen = window::BackgroundDC(dc);
			ReleaseDC(h, dc);
			InvalidateRect(h, NULL, TRUE);
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

void draw_fonts(HWND h, HDC dc, std::vector<font::EnumFontInfo> & ff, size_t skip)
{
	HBRUSH const frame_brush = (HBRUSH) GetStockObject(DC_BRUSH);
	SIZE const frame_extra = {3, 2};
	SIZE const padding = {8, 64};
	SIZE client_size;
	// lib::window::get_inner_size(WindowFromDC(dc), client_size);
	lib::window::get_inner_size(h, client_size);
	// PRINT_VAR(client_size.cx, "%ld");
	// PRINT_VAR(client_size.cy, "%ld");
	int const bottom_y = client_size.cy - padding.cy;

	int y = 0;

	for (size_t i=skip; i<ff.size(); ++i)
	{
		// font::print_font_info(ff[i]);

		char const * text = (char const *) ff[i].elfe.elfFullName;
		size_t const text_len = strlen(text);

		lib::font::EnumFontInfoLoader efil(dc, ff[i]);

		RECT tr = {padding.cx, padding.cy + y, 0, 0};
		snippets::calc_text_rect_2(tr, dc, text, text_len);

		int const next_y = tr.bottom - padding.cy + frame_extra.cy + 1;

		if (next_y > bottom_y) break;

		InflateRect(&tr, frame_extra.cx, frame_extra.cy);
		SetDCBrushColor(dc, RGB(100,100,100));
		FrameRect(dc, &tr, frame_brush);

		RECT rc = {padding.cx, padding.cy + y, 0, 0};
		// lib::font::draw_font_label(dc, rc, ff[i]);
		ExtTextOut(dc, rc.left, rc.top,
			0, nullptr, text, text_len, nullptr);

		y = next_y;
	}
}

void quick_draw(HDC dc, int x, int y, char const * text, size_t text_len, int text_height, COLORREF text_color)
{
	HFONT hf = CreateFont(text_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, nullptr);

	SaveDC(dc);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, text_color);
	SelectObject(dc, hf);
	TextOut(dc, x, y, text, text_len);
	RestoreDC(dc, -1);

	DeleteObject(hf);
}

void draw_info(HDC dc)
{
	char const * const info_text = "( use mouse-wheel to scroll )";
	size_t const info_text_len = strlen(info_text);
	COLORREF const text_color = RGB(200,100,100);

	SIZE client_size;
	lib::window::get_inner_size(WindowFromDC(dc), client_size);

	quick_draw(dc, 8, 8, info_text, info_text_len, 42, text_color);
}
