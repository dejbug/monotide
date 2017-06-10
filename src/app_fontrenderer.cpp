#include "app_fontrenderer.h"
#include <stdexcept>
#ifndef NDEBUG
	#include <stdio.h>
#endif
#include "common.h"
#include "macros.h"
#include "snippets.h"


FontRenderWorker::Job::Job(size_t index)
		: index(index)
{
}


FontRenderWorker::FontRenderWorker(
		std::vector<font::EnumFontInfo> & fonts)
		: hwnd(nullptr), fonts(fonts)
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
	draw_fonts(index);
	// offscreen.flip();
	needs_flip = true;
}

void FontRenderWorker::on_parent_resize()
{
	HDC hdc = GetDC(hwnd);
	offscreen = window::BackgroundDC(hdc);
	ReleaseDC(hwnd, hdc);
}

void FontRenderWorker::queue(size_t index)
{
	EnterCriticalSection(&mutex);
	jobs.push_back(Job(index));
	LeaveCriticalSection(&mutex);
	SetEvent(queue_event);
}


char const * FontRenderWorker::get_msg() const
{
	return msg ? msg : "";
}

size_t FontRenderWorker::get_page_next_count() const
{
	return count_rendered ? count_rendered-1 : 0;
}

size_t FontRenderWorker::get_page_prev_count() const
{
	return 10;
}

void draw_frame(HDC dc, RECT & text_rc, SIZE const & frame_padding,
		COLORREF color=0, bool rc_adjust=false)
{
	HBRUSH const frame_brush = (HBRUSH) GetStockObject(DC_BRUSH);
	InflateRect(&text_rc, frame_padding.cx, frame_padding.cy);
	SetDCBrushColor(dc, color);
	FrameRect(dc, &text_rc, frame_brush);
	if (!rc_adjust)
		InflateRect(&text_rc, -frame_padding.cx, -frame_padding.cy);
}

void FontRenderWorker::draw_fonts(size_t skip)
{
	HDC dc = offscreen.handle;

	if (fonts.size() != fonts_size.size())
	{
		fonts_size.clear();
		fonts_size.resize(fonts.size());
		// PRINT_VAR(fonts_size.size(), "%d");
	}

	snippets::RowIndexDrawer rid;
	rid.set_digits_from_max_index(fonts.size());

	/// A row, at least, must accomodate the index (comfortably) .
	/// The index is the row's "line number" drawn in column #0 .
	int const min_row_height = rid.get_height(1.5f);

	/// The extra space between columns and rows .
	int const row_spacing = 0;
	int const col_spacing = 16;

	/// Extra space between frames and contents .
	SIZE const client_padding = {8, 8};
	SIZE const frame_padding = {2 , 2};

	SIZE client_size;
	lib::window::get_inner_size(hwnd, client_size);

	/// The rightmost, bottommost points after which nothing
	/// SHOULD be drawn.
	SIZE const cutoff = {
		client_size.cx - client_padding.cx,
		client_size.cy - client_padding.cy};

	count_rendered = 0;
	int y = client_padding.cy;

	for (size_t i=skip; i<fonts.size(); ++i, ++count_rendered)
	{
		char const * text = (char const *) fonts[i].elfe.elfFullName;
		size_t const text_len = strlen(text);

		/// Select this font into the device context, so we can
		/// measure it, etc. .
		if (preferredFontHeight)
		{
			fonts[i].elfe.elfLogFont.lfHeight = 42;
			fonts[i].elfe.elfLogFont.lfWidth = 0;
		}
		lib::font::EnumFontInfoLoader efil(offscreen.handle, fonts[i]);

		RECT text_rc = {client_padding.cx, y, 0, 0};
		snippets::calc_text_rect_2(text_rc, dc, text, text_len);

		/// Calculate the y-advance .
		int const text_height = text_rc.bottom - text_rc.top;
		int const frame_height = text_height + frame_padding.cy * 2;
		int const row_height = frame_height > min_row_height ?
			frame_height : min_row_height;
		int const row_height_collapsed = row_height - 1;
		int const next_y = y + row_height_collapsed + row_spacing;

		/// Is there room to draw this item or are we done ?
		if (next_y > cutoff.cy) break;

		/// Vertical centering offsets.
		int const index_offset_y =
			(row_height - rid.get_height()) / 2;
		int const text_offset_y =
			text_height > min_row_height ? 0 :
				(min_row_height - text_height) / 2;

		RECT prefix_rc = {client_padding.cx, y + index_offset_y, 0, 0};
		rid.draw(offscreen.handle, prefix_rc, i+1);

		int const off_text_x = prefix_rc.right + col_spacing;
		OffsetRect(&text_rc, off_text_x, text_offset_y);

		SelectObject(offscreen.handle, GetStockObject(DC_PEN));
		SetDCPenColor(offscreen.handle, RGB(100,100,100));

		/// Line linking index to frame .
		int const y_line = y + int(row_height / 2.0f);
		MoveToEx(offscreen.handle, prefix_rc.right, y_line, nullptr);
		LineTo(offscreen.handle, text_rc.left - frame_padding.cx, y_line);

		// SetBkColor(offscreen.handle, RGB(10,10,10));
		// SetTextColor(offscreen.handle, RGB(200,200,200));

		TextOut(offscreen.handle, text_rc.left, text_rc.top, text, text_len);
		draw_frame(offscreen.handle, text_rc, frame_padding, RGB(100,100,100));

		if(!fonts_size[i].cx || !fonts_size[i].cy)
			fonts_size[i] = SIZE{
				(text_rc.right-text_rc.left) + frame_padding.cx * 2,
				(text_rc.bottom-text_rc.top) + frame_padding.cy * 2};

		// SelectObject(offscreen.handle, GetStockObject(DC_PEN));
		// SetDCPenColor(offscreen.handle, RGB(100,100,100));

		/// A vertical line along the left side of the frame .
		MoveToEx(offscreen.handle, text_rc.left - 4, text_rc.top, nullptr);
		LineTo(offscreen.handle, text_rc.left - 4, text_rc.bottom);

		y = next_y;
	}
}
