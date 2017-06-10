#include <windows.h>
#include <windowsx.h>
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


#define HANDLE_WM_FR_MESSAGE_UPDATE(h,w,l,fn) ((fn)(h),(LRESULT)0)

bool const draw_while_thumb_tracking = false;
bool const only_TTF = false;

static std::vector<font::EnumFontInfo> ff;
static UINT rows_per_scroll = 1;
static snippets::ScrollBar vbar(SB_VERT);
static FontRenderWorker font_renderer(ff);


bool wm_create(HWND h, LPCREATESTRUCT cs)
{
	vbar.parent = h;
	font_renderer.hwnd = h;
	// font_renderer.preferredFontHeight = 42;

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

	font_renderer.start();

	return true;
}

void wm_paint(HWND h)
{
	PAINTSTRUCT ps;
	BeginPaint(h, &ps);

	RECT client_rect;
	GetClientRect(h, &client_rect);
	FillRect(ps.hdc, &client_rect, (HBRUSH)(COLOR_MENU+1));
	// PostMessage(h, WM_FR_MESSAGE_UPDATE, 0, 0);

	EndPaint(h, &ps);

	font_renderer.queue(vbar.index);
}

void wm_keydown(HWND h, UINT key, BOOL, int repeatCount, UINT flags)
{
	switch(key)
	{
		case VK_PRIOR:
		{
			// int const steps = font_renderer.count_rendered >=
			// 	rows_per_scroll ? rows_per_scroll :
			// 	font_renderer.count_rendered;
			// if (vbar.scroll(-steps))
			// 	InvalidateRect(h, NULL, TRUE);
			if (vbar.scroll(-font_renderer.get_page_prev_count()))
				InvalidateRect(h, NULL, TRUE);
			break;
		}

		case VK_NEXT:
		{
			// int const steps = font_renderer.count_rendered > 1 ?
			// 	font_renderer.count_rendered-1 :
			// 	font_renderer.count_rendered;
			// if (vbar.scroll(steps))
			// 	InvalidateRect(h, NULL, TRUE);
			if (vbar.scroll(font_renderer.get_page_next_count()))
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
}

void wm_vscroll(HWND h, HWND bar, UINT nScrollCode, int nPos)
{
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
			if (!font_renderer.count_rendered ||
					vbar.scroll(+font_renderer.count_rendered-1))
				InvalidateRect(h, NULL, TRUE);
			break;

		case SB_THUMBTRACK:
			vbar.index = nPos;
			if (draw_while_thumb_tracking)
			{
				vbar.update();
				font_renderer.queue(vbar.index);
			}
			break;

		case SB_THUMBPOSITION:
			vbar.update();
			font_renderer.queue(vbar.index);
			break;
	}
}

void wm_nclbuttondown(HWND h, BOOL dblclk, int x, int y, UINT nHittest)
{
	if (!draw_while_thumb_tracking && HTVSCROLL == nHittest)
	{
		PostMessage(h, WM_FR_MESSAGE_UPDATE, 0, 0);
	}
	FORWARD_WM_NCLBUTTONDOWN(h, dblclk, x, y, nHittest, DefWindowProc);
}

void wm_mousewheel(HWND h, int x, int y, int zDelta, UINT fwKeys)
{
	if (zDelta > 0 && vbar.scroll(-rows_per_scroll))
		InvalidateRect(h, NULL, TRUE);

	else if (zDelta < 0 && vbar.scroll(+rows_per_scroll))
		InvalidateRect(h, NULL, TRUE);
}

void wm_size(HWND h, UINT fwSizeType , int cx, int cy)
{
	font_renderer.on_parent_resize();
	InvalidateRect(h, NULL, TRUE);
}

void wm_destroy(HWND h)
{
	font_renderer.stop();
#ifdef FR_WAIT_AT_EXIT
	font_renderer.wait(FR_WAIT_AT_EXIT);
#endif
	PostQuitMessage(0);
}

void wm_fr_message_update(HWND h)
{
	RECT client_rect;
	GetClientRect(h, &client_rect);
	HDC dc = GetDC(h);
	FillRect(dc, &client_rect, (HBRUSH)(COLOR_MENU+1));
#ifdef DEBUG_FR_DELAY
	lib::window::quick_draw(dc, 8, 8,
		font_renderer.get_msg(), -1, 42, RGB(100,100,100));
#endif
	ReleaseDC(h, dc);
}

LRESULT CALLBACK MainFrameProc(HWND h, UINT m, WPARAM wParam, LPARAM lParam)
{
	switch(m)
	{
		default: return DefWindowProc(h, m, wParam, lParam);

		case WM_CLOSE: DestroyWindow(h); return 0;
		case WM_ERASEBKGND: return 0;

		HANDLE_MSG(h, WM_CREATE, wm_create);
		HANDLE_MSG(h, WM_DESTROY, wm_destroy);
		HANDLE_MSG(h, WM_SIZE, wm_size);
		HANDLE_MSG(h, WM_PAINT, wm_paint);
		HANDLE_MSG(h, WM_MOUSEWHEEL, wm_mousewheel);
		HANDLE_MSG(h, WM_KEYDOWN, wm_keydown);
		HANDLE_MSG(h, WM_VSCROLL, wm_vscroll);
		HANDLE_MSG(h, WM_NCLBUTTONDOWN, wm_nclbuttondown);

		HANDLE_MSG(h, WM_FR_MESSAGE_UPDATE, wm_fr_message_update);
	}
}

int WINAPI WinMain(HINSTANCE i, HINSTANCE, LPSTR, int iCmdShow)
{
	char const * const frame_cn = "monotide-MAINFRAME";

	window::create_class(frame_cn, MainFrameProc, i);

	HWND frame = window::create_frame(frame_cn, i);
	SetWindowText(frame, "monotide-redux");
	window::set_size(frame, 600, 800);
	window::center_to_parent(frame);

	ShowWindow(frame, iCmdShow);
	UpdateWindow(frame);

	MSG msg;
	window::run_main_loop(msg);
	return msg.wParam;
}
