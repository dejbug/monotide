#ifndef MONOTIDE_APP_FONTRENDERER_H
#define MONOTIDE_APP_FONTRENDERER_H

#include <tchar.h>
#include <windows.h>
#include <vector>
#include "lib_worker.h"
#include "lib_window.h"
#include "lib_font.h"
#include "snippets.h"

using namespace lib;

struct FontDrawCache
{
	std::vector<SIZE> sizes;

	void ensure_capacity(std::vector<font::EnumFontInfo> &);
	void precalc(std::vector<font::EnumFontInfo> &,
		long preferredFontHeight=0);
	void get_size(OUT RECT &, size_t index, HDC, LPCTSTR, size_t);
};

struct FontRenderWorker
		: worker::Worker
{
	HWND hwnd = nullptr;
	LONG preferredFontHeight = 0;
	int min_row_height = 0;

	size_t first_index = 0;
	size_t count_rendered = 0;

	/// The extra space between columns and rows .
	int const row_spacing = 12;
	int const col_spacing = 16;

	/// Extra space between frames and contents .
	SIZE const client_padding = {8, 8};
	SIZE const frame_padding = {3 , 3};

	FontDrawCache draw_cache;

	struct Job
	{
		size_t index;

		Job(size_t);
	};

	FontRenderWorker(std::vector<font::EnumFontInfo> &);
	virtual ~FontRenderWorker();

	void recalc_font_sizes();
	void on_parent_resize();

	void queue(size_t);

	LPCTSTR get_msg() const;

	size_t get_page_next_count() const;
	size_t get_page_prev_count() const;

private:
	CRITICAL_SECTION mutex;
	HANDLE queue_event;
	std::vector<Job> jobs;

	LPCTSTR msg = nullptr;

	window::BackgroundDC offscreen;
	std::vector<font::EnumFontInfo> & fonts;

	snippets::RowIndexDrawer rid;

	bool recalcFontSizes = false;

	void draw_fonts(size_t);
	void draw_fonts_ex(size_t first);

	void task();
};

#endif // !MONOTIDE_APP_FONTRENDERER_H
