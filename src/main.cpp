#include <windows.h>
#include <stdio.h>
#include <vector>
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
	int const margin = 8;

	SIZE client_size;
	lib::window::get_inner_size(WindowFromDC(dc), client_size);

	int y = 0;
	// INT * char_widths = new INT[256];
	// INT * text_offsets = new INT[1024];

	for (size_t i=skip; i<ff.size(); ++i)
	{
		if (y + ff[i].elfe.elfLogFont.lfHeight > client_size.cy - margin)
			break;

		// printf(" %3d | %s | %s | %s | %d | %s\n", i+1, ft,
		// 	ff[i].elfe.elfLogFont.lfFaceName,
		// 	ff[i].elfe.elfStyle,
		// 	ff[i].elfe.elfLogFont.lfCharSet,
		// 	ff[i].elfe.elfScript);

		RECT rc = {margin, margin + y, 0, 0};
		snippets::draw_font_label_1(dc, rc, ff[i]);

		// HBRUSH const text_box_brush = (HBRUSH) GetStockObject(DC_BRUSH);
		// SetDCBrushColor(dc, RGB(100,100,100));
		// FrameRect(dc, &text_box_rect, text_box_brush);

		// y += ff[i].elfe.elfLogFont.lfHeight;
		y = rc.bottom - rc.left + 1;
	}

	// delete[] text_offsets;
	// delete[] char_widths;
}
