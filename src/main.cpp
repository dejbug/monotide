#include <windows.h>
#include <stdio.h>
#include <stdexcept>
#include <vector>
#include "common.h"
#include "macros.h"
#include "snippets.h"
#include "lib_window.h"
#include "lib_font.h"
#include "app_fontrenderer.h"

using namespace lib;


bool const draw_while_thumb_tracking = false;
bool const only_TTF = false;


LRESULT CALLBACK MainFrameProc(HWND, UINT, WPARAM, LPARAM);

void draw_info(HDC, char const * const info_text=
	"( use mouse-wheel to scroll )");


int WINAPI WinMain(HINSTANCE i, HINSTANCE, LPSTR, int iCmdShow)
{
	char const * const class_name = "MAINFRAME";

	window::create_class(class_name, MainFrameProc, i);

	HWND frame = window::create_frame(class_name, i);
	SetWindowText(frame, "monotide-redux");
	window::set_size(frame, 600, 800);
	window::center_to_parent(frame);

	ShowWindow(frame, iCmdShow);
	UpdateWindow(frame);

	MSG msg;
	window::run_main_loop(msg);
	return msg.wParam;
}

LRESULT CALLBACK MainFrameProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	static std::vector<font::EnumFontInfo> ff;
	static UINT rows_per_scroll = 1;
	static snippets::ScrollBar vbar(h, SB_VERT);
	static size_t count_rendered = 0;
	static FontRenderWorker font_renderer(h, ff);

	switch(m)
	{
		default: break;

		case WM_FR_MESSAGE_UPDATE:
		{
			RECT client_rect;
			GetClientRect(h, &client_rect);
			HDC dc = GetDC(h);
			FillRect(dc, &client_rect, (HBRUSH)(COLOR_MENU+1));
#ifdef DEBUG_FR_DELAY
			draw_info(dc, font_renderer.get_msg());
#endif
			ReleaseDC(h, dc);
			return 0;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(h, &ps);

			RECT client_rect;
			GetClientRect(h, &client_rect);
			FillRect(ps.hdc, &client_rect, (HBRUSH)(COLOR_MENU+1));
			// PostMessage(h, WM_FR_MESSAGE_UPDATE, 0, 0);

			EndPaint(h, &ps);

			font_renderer.queue(vbar.index, count_rendered);

			return 0;
		}

		case WM_ERASEBKGND:
			return 0;

		case WM_NCLBUTTONDOWN:
		{
			INT const nHittest = (INT) w;
			// POINTS const pts = MAKEPOINTS(l);

			if (!draw_while_thumb_tracking && HTVSCROLL == nHittest)
			{
				PostMessage(h, WM_FR_MESSAGE_UPDATE, 0, 0);
			}
			break;
		}

		case WM_VSCROLL:
		{
			int const nScrollCode = (int) LOWORD(w);
			short int const nPos = (short int) HIWORD(w);
			// HWND const hwndScrollBar = (HWND) l;

			switch(nScrollCode)
			{
				case SB_LINEUP:
					if (vbar.scroll(-1))
						InvalidateRect(h, NULL, TRUE);
					break;

				case SB_LINEDOWN:
					if (vbar.scroll(+1))
						InvalidateRect(h, NULL, TRUE);
					break;

				case SB_PAGEUP:
					if (vbar.scroll(-rows_per_scroll))
						InvalidateRect(h, NULL, TRUE);
					break;

				case SB_PAGEDOWN:
					if (vbar.scroll(+count_rendered-1))
						InvalidateRect(h, NULL, TRUE);
					break;

				case SB_THUMBTRACK:
					vbar.index = nPos;
					if (draw_while_thumb_tracking)
					{
						vbar.update();
						font_renderer.queue(vbar.index, count_rendered);
					}
					break;

				case SB_THUMBPOSITION:
					vbar.update();
					font_renderer.queue(vbar.index, count_rendered);
					break;

			}

			return 0;
		}

		case WM_MOUSEWHEEL:
		{
			short const zDelta = (short) HIWORD(w);

			if (zDelta > 0 && vbar.scroll(-rows_per_scroll))
				InvalidateRect(h, NULL, TRUE);

			else if (zDelta < 0 && vbar.scroll(+rows_per_scroll))
				InvalidateRect(h, NULL, TRUE);

			return 0;
		}

		case WM_SIZE:
		{
			font_renderer.on_parent_resize();
			InvalidateRect(h, NULL, TRUE);
			return 0;
		}

		case WM_CREATE:
		{
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES,
				0, &rows_per_scroll, 0);

			HDC dc = GetDC(h);
			font::list_fonts(ff, ANSI_CHARSET, only_TTF, dc);
			font::sort_fonts(ff);
			ReleaseDC(h, dc);

#ifndef NDEBUG
			printf(" %d fonts found\n", ff.size());
#endif

			vbar.set_count(ff.size());

			// TODO: Do all the setup we can as early as possible.
			// FIXME: Move offscreen into font_renderer.
			// font_renderer.setup(h, ff);
			font_renderer.start();

			return 0;
		}

		case WM_KEYDOWN:
		{
			switch(w)
			{
				case VK_PRIOR:
				{
					int const steps = count_rendered >= rows_per_scroll ?
						rows_per_scroll : count_rendered;
					if (vbar.scroll(-steps))
						InvalidateRect(h, NULL, TRUE);
					break;
				}

				case VK_NEXT:
				{
					int const steps = count_rendered > 1 ?
						count_rendered-1 : count_rendered;
					if (vbar.scroll(steps))
						InvalidateRect(h, NULL, TRUE);
					break;
				}

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
			font_renderer.stop();
#ifdef FR_WAIT_AT_EXIT
			font_renderer.wait(FR_WAIT_AT_EXIT);
#endif
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(h, m, w, l);
}

void draw_info(HDC dc, char const * const info_text)
{
	size_t const info_text_len = strlen(info_text);
	// COLORREF const text_color = RGB(200,100,100);
	COLORREF const text_color = RGB(100,100,100);

	SIZE client_size;
	lib::window::get_inner_size(WindowFromDC(dc), client_size);

	lib::window::quick_draw(dc, 8, 8, info_text, info_text_len,
		42, text_color);
}
