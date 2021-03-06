#include "app_fontrenderer.h"
#include <stdexcept>
#ifndef NDEBUG
	#include <stdio.h>
#endif
#include "common.h"
// #include "macros.h"

#ifndef NDEBUG
// #define _APP_FONTRENDERER_LOG_JOBS
// #define _APP_FONTRENDERER_LOG_RENDER
// #define _APP_FONTRENDERER_LOG_PAGES
// #define _APP_FONTRENDERER_LOG_CACHE
// #define _APP_FONTRENDERER_DEBUG_SLOW_DRAW	 	500
// #define _APP_FONTRENDERER_DEBUG_TICK_DELAY		500
#endif

void FontDrawCache::ensure_capacity(std::vector<font::EnumFontInfo> & fonts)
{
	if (fonts.size() != sizes.size())
	{
		sizes.clear();
		sizes.resize(fonts.size());
		// PRINT_VAR(sizes.size(), "%d");
	}
}

void FontDrawCache::precalc(std::vector<font::EnumFontInfo> & fonts,
	long preferredFontHeight)
{
	sizes.resize(fonts.size());

	precalced = false;

	HDC dc = GetDC(nullptr);

	for (size_t i=0; i<fonts.size(); ++i)
	{
		LPCTSTR text = (LPCTSTR) fonts[i].elfe.elfFullName;
		size_t const text_len = _tcslen(text);

		if (preferredFontHeight)
		{
			fonts[i].elfe.elfLogFont.lfHeight = preferredFontHeight;
			fonts[i].elfe.elfLogFont.lfWidth = 0;
		}

		lib::font::EnumFontInfoLoader efil(dc, fonts[i]);

		RECT text_rc = {0, 0, 0, 0};
		snippets::calc_text_rect_2(text_rc, dc, text, text_len);

		sizes[i] = SIZE{
			(text_rc.right-text_rc.left),
			(text_rc.bottom-text_rc.top)};
	}

	ReleaseDC(nullptr, dc);

	precalced = true;
}

void FontDrawCache::get_size(RECT & rc, size_t index, HDC dc,
		LPCTSTR text, size_t text_len)
{
	if(!sizes[index].cx || !sizes[index].cy)
	{
		snippets::calc_text_rect_2(rc, dc, text, text_len);
		sizes[index] = SIZE{rc.right - rc.left, rc.bottom - rc.top};
#ifdef _APP_FONTRENDERER_LOG_CACHE
		_tprintf(_T("cached\n"));
#endif
	}
	else
	{
		rc.right = rc.left + sizes[index].cx;
		rc.bottom = rc.top + sizes[index].cy;
#ifdef _APP_FONTRENDERER_LOG_CACHE
		_tprintf(_T("cache hit\n"));
#endif
	}
}

FontRenderWorker::Job::Job(size_t index)
		: index(index)
{
}

FontRenderWorker::Job::~Job()
{
#ifdef _APP_FONTRENDERER_LOG_JOBS
	_tprintf(_T("~Job #%d\n"), index);
#endif
}

FontRenderWorker::FontRenderWorker(
		std::vector<font::EnumFontInfo> & fonts)
		: hwnd(nullptr), fonts(fonts)
{
	/// A row, at least, must accomodate the index (comfortably) .
	/// The index is the row's "line number" drawn in column #0 .
	if (!min_row_height)
	{
		// min_row_height = (size_t) lib::font::get_sysfont_height() * 1.5f;
		min_row_height = (size_t) lib::font::get_sysfont_height();
	}
}

FontRenderWorker::~FontRenderWorker()
{
}

void FontRenderWorker::stop()
{
	worker::Worker::stop();
	jobs.notify();
}

void FontRenderWorker::task()
{
#ifdef _APP_FONTRENDERER_LOG_RENDER
	_tprintf(_T("\n"));
#endif

	static bool needs_flip = true;
	bool skip = true;

	size_t index = 0;

#ifdef _APP_FONTRENDERER_LOG_JOBS
	size_t jobs_dropped = 0;
#endif

#ifdef _APP_FONTRENDERER_DEBUG_TICK_DELAY
	_tprintf(_T(" [ font renderer %08x ] tick\n"), (size_t) this);
#endif

	// EnterCriticalSection(&mutex);
	Job * job = jobs.get_back();
	if (job)
	{
		if (!msg || !*msg) msg = _T("rendering...");
		else msg = _T("rendering... ( stop scrolling! :)");

#ifdef _APP_FONTRENDERER_LOG_JOBS
		jobs_dropped = jobs.get_count() - 1;
#endif

		skip = false;
		index = job->index;
		jobs.clear();

		bool const already_rendered = (index == last_index_rendered);

		if (already_rendered) skip = needs_flip = true;
		else
		{
#ifdef _APP_FONTRENDERER_DEBUG_SLOW_DRAW
			PostMessage(hwnd, WM_FR_MESSAGE_UPDATE, 0, 0);
#endif
		}
	}
	// LeaveCriticalSection(&mutex);

#ifdef _APP_FONTRENDERER_LOG_RENDER
	_tprintf(_T("fr == index %d skip %s needs_flip %s\n"), index, skip ? _T("Y") : _T("N"), needs_flip ? _T("Y") : _T("N"));
#endif

	if (skip)
	{
#ifdef _APP_FONTRENDERER_LOG_RENDER
		_tprintf(_T("fr -- skipping"));
#endif
		msg = _T("");
		if (needs_flip)
		{
#ifdef _APP_FONTRENDERER_LOG_RENDER
			_tprintf(_T(" & flipping"));
#endif
			offscreen.flip();
			needs_flip = false;
		}
#ifdef _APP_FONTRENDERER_LOG_RENDER
		_tprintf(_T("\n"));
#endif

#ifdef _APP_FONTRENDERER_DEBUG_TICK_DELAY
		jobs.wait(_APP_FONTRENDERER_DEBUG_TICK_DELAY);
#else
		jobs.wait(INFINITE);
#endif
		return;
	}

#ifdef _APP_FONTRENDERER_LOG_JOBS
	_tprintf(_T(" [ jobs dropped ] %d\n"), jobs_dropped);
#endif

#ifdef _APP_FONTRENDERER_DEBUG_SLOW_DRAW
	Sleep(_APP_FONTRENDERER_DEBUG_SLOW_DRAW);
#endif

#ifdef _APP_FONTRENDERER_LOG_RENDER
	_tprintf(_T("fr -- drawing\n"));
#endif
	offscreen.clear(COLOR_MENU);
	draw_fonts(index);
	// draw_fonts_ex(index);
	last_index_rendered = index;
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
	jobs.push_back(new Job(index));
}

LPCTSTR FontRenderWorker::get_msg() const
{
	return msg ? msg : _T("");
}

size_t FontRenderWorker::get_page_next_count() const
{
	return count_rendered ? count_rendered-1 : 0;
}

size_t FontRenderWorker::get_page_prev_count() const
{
#ifdef _APP_FONTRENDERER_LOG_PAGES
	_tprintf(_T("last_index_rendered %u\n"), last_index_rendered);
#endif

	if ((size_t)-1 == last_index_rendered) return 0;
	if (0 == last_index_rendered) return 0;

	size_t font_count = 0;

	if (draw_cache.precalced)
	{
		SIZE client_size;
		lib::window::get_inner_size(hwnd, client_size);

		int const cutoff_cy = client_size.cy - client_padding.cy;

		long next_y = client_padding.cy;

		for (size_t i=0; i<=last_index_rendered; ++i)
		{
			size_t const index = last_index_rendered-i;
#ifdef _APP_FONTRENDERER_LOG_PAGES
			_tprintf(_T("index %u\n"), index);
#endif
			int const font_height = draw_cache.sizes[index].cy;
			Y_advance y_advance(font_height, frame_padding, min_row_height, row_spacing);
			next_y += y_advance.y;
#ifdef _APP_FONTRENDERER_LOG_PAGES
			_tprintf(_T("next_y %ld\n"), next_y);
#endif
			if (next_y > cutoff_cy) break;

			++font_count;
		}

		if (font_count > 0) --font_count;
#ifdef _APP_FONTRENDERER_LOG_PAGES
		_tprintf(_T("font_count %u\n"), font_count);
#endif
	}

	return font_count;
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

void on_draw_font(HDC dc, size_t /*i*/,
		RECT & rc, LPCTSTR text, size_t text_len)
{
	// PRINT_VAR(text, "%s");
	TextOut(dc, rc.left, rc.top, text, text_len);
	// draw_frame(dc, rc, frame_padding, RGB(100,100,100));

	// Sleep(100);
}

void FontRenderWorker::draw_fonts_ex(size_t first)
{
	SIZE client_size;
	lib::window::get_inner_size(hwnd, client_size);

	SIZE const cutoff = {
		client_size.cx - client_padding.cx,
		client_size.cy - client_padding.cy};

	first_index = first;
	count_rendered = 0;
	int y = client_padding.cy;

	for (size_t i=first; i<fonts.size(); ++i, ++count_rendered)
	{
		if (!jobs.is_empty())
		{
#ifdef _APP_FONTRENDERER_LOG_JOBS
			_tprintf(_T("jobs added while rendering last job\n"));
#endif
			break;
		}

		LPCTSTR text = (LPCTSTR) fonts[i].elfe.elfFullName;
		size_t const text_len = _tcslen(text);

		int const next_y = y + fonts[i].elfe.elfLogFont.lfHeight + row_spacing;

		if (next_y > cutoff.cy) break;

		if (preferredFontHeight)
		{
			fonts[i].elfe.elfLogFont.lfHeight = preferredFontHeight;
			fonts[i].elfe.elfLogFont.lfWidth = 0;
		}
		lib::font::EnumFontInfoLoader efil(offscreen.handle, fonts[i]);

		RECT text_rc = {client_padding.cx, y, 0, 0};
		draw_cache.get_size(text_rc, i, offscreen.handle, text, text_len);
		on_draw_font(offscreen.handle, i, text_rc, text, text_len);
		draw_frame(offscreen.handle, text_rc, frame_padding, RGB(100,100,100));

		y = next_y;
	}
}

__attribute__((unused)) static int calc_y_advance(RECT text_rc, SIZE frame_padding, int min_row_height, int row_spacing)
{
	/// Calculate the y-advance .
	int const text_height = text_rc.bottom - text_rc.top;
	int const frame_height = text_height + frame_padding.cy * 2;
	int const row_height = frame_height > min_row_height ?
		frame_height : min_row_height;
	int const row_height_collapsed = row_height - 1;
	int const y = row_height_collapsed + row_spacing;
	return y;
}

void FontRenderWorker::draw_fonts(size_t skip)
{
	rid.set_digits_from_max_index(fonts.size());

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
		LPCTSTR text = (LPCTSTR) fonts[i].elfe.elfFullName;
		size_t const text_len = _tcslen(text);

		/// Select this font into the device context, so we can
		/// measure it, etc. .
		if (preferredFontHeight)
		{
			fonts[i].elfe.elfLogFont.lfHeight = preferredFontHeight;
			fonts[i].elfe.elfLogFont.lfWidth = 0;
		}
		lib::font::EnumFontInfoLoader efil(offscreen.handle, fonts[i]);

		RECT text_rc = {client_padding.cx, y, 0, 0};
		draw_cache.get_size(text_rc, i, offscreen.handle, text, text_len);
		Y_advance y_advance(text_rc, frame_padding, min_row_height, row_spacing);
		int const next_y = y + y_advance.y;

		/// Is there room to draw this item or are we done ?
		if (next_y > cutoff.cy) break;

		/// Vertical centering offsets.
		int const index_offset_y =
			(y_advance.row_height - rid.get_height()) / 2;
		int const text_offset_y =
			y_advance.text_height > min_row_height ? 0 :
				(min_row_height - y_advance.text_height) / 2;

		RECT prefix_rc = {client_padding.cx, y + index_offset_y, 0, 0};
		rid.draw(offscreen.handle, prefix_rc, i+1);

		int const off_text_x = prefix_rc.right + col_spacing;
		OffsetRect(&text_rc, off_text_x, text_offset_y);

		SelectObject(offscreen.handle, GetStockObject(DC_PEN));
		SetDCPenColor(offscreen.handle, RGB(100,100,100));

		/// Line linking index to frame .
		int const y_line = y + int(y_advance.row_height / 2.0f);
		MoveToEx(offscreen.handle, prefix_rc.right, y_line, nullptr);
		LineTo(offscreen.handle, text_rc.left - frame_padding.cx, y_line);

		TextOut(offscreen.handle, text_rc.left, text_rc.top, text, text_len);
		draw_frame(offscreen.handle, text_rc, frame_padding,
			RGB(100,100,100));

		/// A vertical line along the left side of the frame .
		MoveToEx(offscreen.handle, text_rc.left - 4, text_rc.top, nullptr);
		LineTo(offscreen.handle, text_rc.left - 4, text_rc.bottom);

		y = next_y;
	}
}
