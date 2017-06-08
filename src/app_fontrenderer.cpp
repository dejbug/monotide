#include "app_fontrenderer.h"
#include <stdexcept>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include "common.h"
#include "snippets.h"


FontRenderWorker::Job::Job(size_t index, size_t & count_rendered)
		: index(index), count_rendered(count_rendered)
{
}


FontRenderWorker::FontRenderWorker(HWND h,
		std::vector<font::EnumFontInfo> & ff)
		: hwnd(h), fonts(ff)
{
	InitializeCriticalSection(&mutex);
	queue_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!queue_event) throw std::runtime_error("FontRenderWorker :"
		" unable to create queue_event");
}

FontRenderWorker::~FontRenderWorker()
{
	CloseHandle(queue_event);
	DeleteCriticalSection(&mutex);
}

void FontRenderWorker::task()
{
	static bool needs_flip = true;
	bool skip = true;

	size_t index = 0;
	size_t * count_rendered = nullptr;

#ifndef NDEBUG
	size_t jobs_dropped = 0;
#endif

#ifdef FR_DEBUG_TICK_DELAY
	printf(" [ font renderer %08x ] tick\n", (size_t) this);
#endif

	EnterCriticalSection(&mutex);
	if (!jobs.empty())
	{
		if (!msg || !*msg) msg = "rendering...";
		else msg = "still rendering... ( stop scrolling! :)";

#ifdef FR_DEBUG_SLOW_DRAW
		PostMessage(hwnd, WM_FR_MESSAGE_UPDATE, 0, 0);
#endif

#ifndef NDEBUG
		jobs_dropped = jobs.size() - 1;
#endif
		skip = false;
		index = jobs.back().index;
		count_rendered = &jobs.back().count_rendered;
		jobs.clear();
	}
	LeaveCriticalSection(&mutex);

	if (skip)
	{
		msg = "";
		if (needs_flip)
		{
			offscreen.flip();
			needs_flip = false;
		}

#ifdef FR_DEBUG_TICK_DELAY
		WaitForSingleObject(queue_event, FR_DEBUG_TICK_DELAY);
#else
		WaitForSingleObject(queue_event, INFINITE);
#endif
		return;
	}

#ifndef NDEBUG
	printf(" [ jobs dropped ] %d\n", jobs_dropped);
#endif

#ifdef FR_DEBUG_SLOW_DRAW
	Sleep(FR_DEBUG_SLOW_DRAW);
#endif

	offscreen.clear(COLOR_MENU);
	draw_fonts(hwnd, offscreen.handle, fonts, index, *count_rendered);
	// offscreen.flip();
	needs_flip = true;
}

void FontRenderWorker::on_parent_resize()
{
	HDC hdc = GetDC(hwnd);
	offscreen = window::BackgroundDC(hdc);
	ReleaseDC(hwnd, hdc);
}

void FontRenderWorker::queue(size_t index, size_t & count_rendered)
{
	EnterCriticalSection(&mutex);
	jobs.push_back(Job(index, count_rendered));
	LeaveCriticalSection(&mutex);
	SetEvent(queue_event);
}


char const * FontRenderWorker::get_msg() const
{
	return msg ? msg : "";
}

void draw_frame(HDC dc, RECT & rc, SIZE const & frame_extra,
		COLORREF color=0, bool rc_adjust=false)
{
	HBRUSH const frame_brush = (HBRUSH) GetStockObject(DC_BRUSH);
	InflateRect(&rc, frame_extra.cx, frame_extra.cy);
	SetDCBrushColor(dc, color);
	FrameRect(dc, &rc, frame_brush);
	if (!rc_adjust)
		InflateRect(&rc, -frame_extra.cx, -frame_extra.cy);
}

void draw_fonts(HWND h, HDC dc, std::vector<font::EnumFontInfo> & ff,
		size_t skip, size_t & count_rendered)
{
	TEXTMETRIC tm;
	SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetrics(dc, &tm);

	SIZE const frame_extra = {3, 2};
	SIZE const padding = {8, 8};
	SIZE client_size;
	lib::window::get_inner_size(h, client_size);
	SIZE const cutoff = {
		client_size.cx - padding.cx,
		client_size.cy - padding.cy};
	int const prefix_gap = 16;
	int const min_step_y = int(tm.tmHeight * 1.5);
	int const gap_row = 0;

	count_rendered = 0;
	int y = padding.cy;
	char prefix[16];

	for (size_t i=skip; i<ff.size(); ++i, ++count_rendered)
	{
		char const * text = (char const *) ff[i].elfe.elfFullName;
		size_t const text_len = strlen(text);

		lib::font::EnumFontInfoLoader efil(dc, ff[i]);

		RECT rc = {padding.cx, y, 0, 0};
		snippets::calc_text_rect_2(rc, dc, text, text_len);

		int const step_y = (rc.bottom - rc.top) + frame_extra.cy * 2 - 1;
		int const step_y_true = step_y > min_step_y ? step_y : min_step_y;
		int const next_y = y + step_y_true + gap_row;

		if (next_y > cutoff.cy) break;

		RECT tr = {padding.cx, y, 0, 0};
		_snprintf(prefix, sizeof(prefix), "%4d", i+1);
		SetBkMode(dc, TRANSPARENT);
		HGDIOBJ old_font = SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
		snippets::text_draw_2(dc, tr, prefix, strlen(prefix));
		SelectObject(dc, old_font);
		SetBkMode(dc, OPAQUE);
		int const offset = tr.right + prefix_gap;
		OffsetRect(&rc, offset, 0);

		// SetBkColor(dc, RGB(10,10,10));
		// SetTextColor(dc, RGB(200,200,200));

		TextOut(dc, rc.left, rc.top, text, text_len);

		draw_frame(dc, rc, frame_extra, RGB(100,100,100));

		y = next_y;
	}
}
