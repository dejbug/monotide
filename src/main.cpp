#include <windows.h>
#include <stdio.h>
#include <stdexcept>
#include <vector>
#include "macros.h"
#include "snippets.h"
#include "lib_window.h"
#include "lib_font.h"

using namespace lib;

#define DEBUG_FR_DELAY 1000

#define WM_FR_MESSAGE_UPDATE	(WM_USER + 1)

bool const draw_while_thumb_tracking = false;

struct FontRenderWorker
		: snippets::Worker
{
	struct Job
	{
		size_t index;
		size_t & count_rendered;

		Job(size_t, size_t &);
	};

	FontRenderWorker();

	void setup(HWND, window::BackgroundDC &,
		std::vector<font::EnumFontInfo> &);
	void queue(size_t, size_t &);
	char const * get_msg() const;

private:
	CRITICAL_SECTION mutex;
	HWND hwnd = nullptr;
	window::BackgroundDC * offscreen = nullptr;
	std::vector<font::EnumFontInfo> * fonts;
	std::vector<Job> jobs;
	char const * msg = nullptr;

	void task();
};

LRESULT CALLBACK MainFrameProc(HWND, UINT, WPARAM, LPARAM);

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

void draw_fonts(HWND, HDC, std::vector<font::EnumFontInfo> &,
	size_t, size_t &);
void draw_info(HDC, char const * const info_text=
	"( use mouse-wheel to scroll )");

LRESULT CALLBACK MainFrameProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	static std::vector<font::EnumFontInfo> ff;
	static UINT rows_per_scroll = 1;
	static window::BackgroundDC offscreen;
	static snippets::ScrollBar vbar(h, SB_VERT);
	static size_t count_rendered = 0;
	static FontRenderWorker font_renderer;

	switch(m)
	{
		default: break;

		case WM_FR_MESSAGE_UPDATE:
		{
			RECT client_rect;
			GetClientRect(h, &client_rect);
			HDC dc = GetDC(h);
			FillRect(dc, &client_rect, (HBRUSH)COLOR_WINDOW);
			draw_info(dc, font_renderer.get_msg());
			ReleaseDC(h, dc);
			return 0;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(h, &ps);

			RECT client_rect;
			GetClientRect(h, &client_rect);
			FillRect(ps.hdc, &client_rect, (HBRUSH)COLOR_WINDOW);
			PostMessage(h, WM_FR_MESSAGE_UPDATE, 0, 0);

			EndPaint(h, &ps);

			// draw_fonts(h, offscreen.handle, ff, vbar.index, count_rendered);
			// offscreen.flip();
			font_renderer.queue(vbar.index, count_rendered);

			return 0;
		}

		case WM_ERASEBKGND:
		{
			InvalidateRect(h, NULL, FALSE);

			// RECT client_rect;
			// GetClientRect(h, &client_rect);
			// HDC dc = GetDC(h);
			// FillRect(dc, &client_rect, (HBRUSH)COLOR_WINDOW);
			// draw_info(dc, "hi");
			// ReleaseDC(h, dc);
			return 0;
		}

		case WM_NCLBUTTONDOWN:
		{
			INT const nHittest = (INT) w;
			// POINTS pts = MAKEPOINTS(l);

			if (!draw_while_thumb_tracking && HTVSCROLL == nHittest)
			{
				// offscreen.clear(0);
				// draw_info(offscreen.handle);
				// offscreen.flip();
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
						InvalidateRect(h, NULL, TRUE);
					}
					break;

				case SB_THUMBPOSITION:
					vbar.update();
					InvalidateRect(h, NULL, TRUE);
					break;

			}

			return 0;
		}

		case WM_MOUSEWHEEL:
		{
			short const zDelta = (short) HIWORD(w);

			// printf("WM_MOUSEWHEEL %d\n", zDelta);

			if (zDelta > 0 && vbar.scroll(-rows_per_scroll))
				InvalidateRect(h, NULL, TRUE);

			else if (zDelta < 0 && vbar.scroll(+rows_per_scroll))
				InvalidateRect(h, NULL, TRUE);

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

			// printf(" %d fonts found\n", ff.size());

			vbar.set_count(ff.size());

			font_renderer.setup(h, offscreen, ff);
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
			font_renderer.wait(1500);
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProc(h, m, w, l);
}

void draw_fonts(HWND h, HDC dc, std::vector<font::EnumFontInfo> & ff,
		size_t skip, size_t & count_rendered)
{
	HBRUSH const frame_brush = (HBRUSH) GetStockObject(DC_BRUSH);
	SIZE const frame_extra = {3, 2};
	SIZE const padding = {8, 64};
	SIZE client_size;
	lib::window::get_inner_size(h, client_size);
	SIZE const cutoff = {
		client_size.cx - padding.cx,
		client_size.cy - padding.cy};

	count_rendered = 0;
	int y = 0;

	for (size_t i=skip; i<ff.size(); ++i, ++count_rendered)
	{
		char const * text = (char const *) ff[i].elfe.elfFullName;
		size_t const text_len = strlen(text);

		lib::font::EnumFontInfoLoader efil(dc, ff[i]);

		RECT tr = {padding.cx, padding.cy + y, 0, 0};
		snippets::calc_text_rect_2(tr, dc, text, text_len);

		int const next_y = tr.bottom - padding.cy + frame_extra.cy + 1;

		if (next_y > cutoff.cy) break;

		InflateRect(&tr, frame_extra.cx, frame_extra.cy);
		// SetDCBrushColor(dc, RGB(200,80,80));
		SetDCBrushColor(dc, RGB(100,100,100));
		FrameRect(dc, &tr, frame_brush);

		RECT rc = {padding.cx, padding.cy + y, 0, 0};
		// lib::font::draw_font_label(dc, rc, ff[i]);
		// SetBkColor(dc, RGB(10,10,10));
		// SetTextColor(dc, RGB(200,200,200));
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

void draw_info(HDC dc, char const * const info_text)
{
	size_t const info_text_len = strlen(info_text);
	// COLORREF const text_color = RGB(200,100,100);
	COLORREF const text_color = RGB(100,100,100);

	SIZE client_size;
	lib::window::get_inner_size(WindowFromDC(dc), client_size);

	quick_draw(dc, 8, 8, info_text, info_text_len, 42, text_color);
}

FontRenderWorker::Job::Job(size_t index, size_t & count_rendered)
		: index(index), count_rendered(count_rendered)
{
}

FontRenderWorker::FontRenderWorker()
{
	InitializeCriticalSection(&mutex);
	// mutex = CreateMutex(nullptr, FALSE, nullptr);
	// if (!mutex) throw std::runtime_error("FontRenderWorker : unable to create mutex");
}

void FontRenderWorker::task()
{
	bool skip = true;

	size_t jobs_dropped = 0;

	size_t index = 0;
	size_t * count_rendered = nullptr;

	printf(" [ font renderer %08x ] tick\n", (size_t) this);

	EnterCriticalSection(&mutex);
	if (!jobs.empty())
	{
		if (!msg || !*msg) msg = "rendering...";
		else msg = "still rendering... ( stop scrolling! :)";
		PostMessage(hwnd, WM_FR_MESSAGE_UPDATE, 0, 0);

		skip = false;
		jobs_dropped = jobs.size() - 1;
		index = jobs.back().index;
		count_rendered = &jobs.back().count_rendered;
		jobs.clear();
	}
	LeaveCriticalSection(&mutex);

	if (skip)
	{
		msg = "";

		offscreen->flip();

		Sleep(100);

		return;
	}

	printf(" [ jobs dropped ] %d\n", jobs_dropped);

#ifdef DEBUG_FR_DELAY
	Sleep(DEBUG_FR_DELAY);
#endif

	offscreen->clear(0);
	draw_fonts(hwnd, offscreen->handle, *fonts, index, *count_rendered);
	// offscreen->flip();
}

void FontRenderWorker::setup(HWND hwnd, window::BackgroundDC & offscreen,
		std::vector<font::EnumFontInfo> & fonts)
{
	EnterCriticalSection(&mutex);
	this->hwnd = hwnd;
	this->offscreen = &offscreen;
	this->fonts = &fonts;
	LeaveCriticalSection(&mutex);
}

void FontRenderWorker::queue(size_t index, size_t & count_rendered)
{
	EnterCriticalSection(&mutex);
	jobs.push_back(Job(index, count_rendered));
	LeaveCriticalSection(&mutex);
}


char const * FontRenderWorker::get_msg() const
{
	return msg ? msg : "";
}

