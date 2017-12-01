#include "fontlist.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdexcept>
#include <vector>
#include "common.h"
#include "snippets.h"
#include "lib_window.h"
#include "lib_font.h"
#include "app_fontrenderer.h"

#ifndef NDEBUG
// #define _FONTLIST_LOG_FONTS_ENUM
// #define _FONTLIST_WAIT_AT_EXIT					1000
// #define _APP_FONTRENDERER_DEBUG_SLOW_DRAW		500
#endif

#define HANDLE_WM_FR_MESSAGE_UPDATE(h,w,l,fn) ((fn)(h),(LRESULT) 0)

struct PrivateData
{
	HWND hwnd = nullptr;
	LONG style = 0;
};

constexpr bool DRAW_WHILE_THUMB_TRACKING = false;
constexpr bool ONLY_TTF = false;
constexpr bool PRECALC_FONT_HEIGHTS = true;
constexpr LONG PREFERRED_FONT_HEIGHT = 0;

static std::vector<font::EnumFontInfo> fonts;
static UINT rows_per_scroll = 1;
static snippets::ScrollBar vbar(SB_VERT);
static FontRenderWorker font_renderer(fonts);


static bool wm_create(HWND h, LPCREATESTRUCT /*cs*/)
{
	PostMessage(h, WM_FR_MESSAGE_UPDATE, 0, 0);

	vbar.parent = h;
	font_renderer.hwnd = h;

	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &rows_per_scroll, 0);

	HDC dc = GetDC(h);
	font::list_fonts(fonts, ANSI_CHARSET, ONLY_TTF, dc);
	font::sort_fonts(fonts);
	ReleaseDC(h, dc);

	if (PREFERRED_FONT_HEIGHT && !PRECALC_FONT_HEIGHTS)
		font_renderer.preferredFontHeight = PREFERRED_FONT_HEIGHT;

	if (PRECALC_FONT_HEIGHTS)
		font_renderer.draw_cache.precalc(fonts, PREFERRED_FONT_HEIGHT);
	else
		font_renderer.draw_cache.ensure_capacity(fonts);

#ifdef _FONTLIST_LOG_FONTS_ENUM
	printf(" %d fonts found\n", fonts.size());
#endif

	vbar.set_count(fonts.size());

	font_renderer.start();

	return true;
}

static void wm_paint(HWND h)
{
	PAINTSTRUCT ps;
	BeginPaint(h, &ps);

	// RECT client_rect;
	// GetClientRect(h, &client_rect);
	// FillRect(ps.hdc, &client_rect, (HBRUSH)(COLOR_MENU+1));

	EndPaint(h, &ps);

	font_renderer.queue(vbar.index);
}

static void wm_keydown(HWND h, UINT key, BOOL, int /*repeatCount*/, UINT /*flags*/)
{
	switch(key)
	{
		case VK_PRIOR:
		{
			if (vbar.scroll(-font_renderer.get_page_prev_count()))
				InvalidateRect(h, NULL, TRUE);
			break;
		}

		case VK_NEXT:
		{
			if (vbar.scroll(font_renderer.get_page_next_count()))
				InvalidateRect(h, NULL, TRUE);
			break;
		}

		case VK_ESCAPE:
			window::close_window(h);
			break;

		case VK_F5:
			InvalidateRect(h, NULL, TRUE);
			UpdateWindow(h);
			break;
	}
}

static void wm_vscroll(HWND h, HWND /*bar*/, UINT nScrollCode, int nPos)
{
	switch(nScrollCode)
	{
		case SB_LINEUP:
			if (vbar.scroll(-1)) InvalidateRect(h, NULL, TRUE);
			else InvalidateRect(h, NULL, FALSE);
			break;

		case SB_LINEDOWN:
			if (vbar.scroll(+1)) InvalidateRect(h, NULL, TRUE);
			else InvalidateRect(h, NULL, FALSE);
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
			if (DRAW_WHILE_THUMB_TRACKING)
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

static void wm_nclbuttondown(HWND h, BOOL dblclk, int x, int y, UINT nHittest)
{
	if (!DRAW_WHILE_THUMB_TRACKING && HTVSCROLL == nHittest)
	{
		PostMessage(h, WM_FR_MESSAGE_UPDATE, 0, 0);
	}
	FORWARD_WM_NCLBUTTONDOWN(h, dblclk, x, y, nHittest, DefWindowProc);
}

static void wm_mousewheel(HWND h, int /*x*/, int /*y*/, int zDelta, UINT /*fwKeys*/)
{
	if (zDelta > 0 && vbar.scroll(-rows_per_scroll))
		InvalidateRect(h, NULL, TRUE);

	else if (zDelta < 0 && vbar.scroll(+rows_per_scroll))
		InvalidateRect(h, NULL, TRUE);
}

static void wm_size(HWND h, UINT /*fwSizeType*/ , int /*cx*/, int /*cy*/)
{
	font_renderer.on_parent_resize();
	InvalidateRect(h, NULL, TRUE);
}

static void wm_destroy(HWND)
{
	font_renderer.stop();
#ifdef _FONTLIST_WAIT_AT_EXIT
	font_renderer.wait(_FONTLIST_WAIT_AT_EXIT);
#endif
	PostQuitMessage(0);
}

static bool wm_nccreate(HWND h, LPCREATESTRUCT lpcs)
{
	FORWARD_WM_NCCREATE(h, lpcs, DefWindowProc);

	PrivateData * pData = new PrivateData;
	if(!pData) return false;
	SetWindowLongPtr(h, 0, (LONG_PTR) pData);
	pData->hwnd = h;
	pData->style = lpcs->style;
	return true;
}

static void wm_ncdestroy(HWND h)
{
	PrivateData * pData = (PrivateData *) GetWindowLongPtr(h, 0);
	SetWindowLongPtr(h, 0, (LONG_PTR) nullptr);
	if (pData) delete pData;
}

static void wm_fr_message_update(HWND h)
{
	RECT client_rect;
	GetClientRect(h, &client_rect);
	HDC dc = GetDC(h);
	FillRect(dc, &client_rect, (HBRUSH)(COLOR_MENU+1));
#ifdef _APP_FONTRENDERER_DEBUG_SLOW_DRAW
	lib::window::quick_draw(dc, 8, 8,
		font_renderer.get_msg(), -1, 42, RGB(100,100,100));
#endif
	ReleaseDC(h, dc);
}

static LRESULT CALLBACK Callback(HWND h, UINT m, WPARAM wParam, LPARAM lParam)
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

		HANDLE_MSG(h, WM_NCCREATE, wm_nccreate);
		HANDLE_MSG(h, WM_NCDESTROY, wm_ncdestroy);

		HANDLE_MSG(h, WM_FR_MESSAGE_UPDATE, wm_fr_message_update);
	}
}

void RegisterFontList()
{
	WNDCLASSEX wc;
	lib::window::init_class(wc);

    // wc.style |= CS_GLOBALCLASS;
	wc.cbWndExtra = sizeof(PrivateData *);
    wc.lpszClassName = WC_FONTLIST;
    wc.lpfnWndProc = Callback;

    lib::window::create_class(wc);
}

void UnegisterFontList()
{
	UnregisterClass(WC_FONTLIST, nullptr);
}
