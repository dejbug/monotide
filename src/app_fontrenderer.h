#pragma once
#include <windows.h>
#include <vector>
#include "lib_worker.h"
#include "lib_window.h"
#include "lib_font.h"

using namespace lib;

struct FontRenderWorker
		: worker::Worker
{
	HWND hwnd = nullptr;
	size_t count_rendered = 0;
	LONG preferredFontHeight = 0;
	bool precalcFontSizes = true;

	/// The extra space between columns and rows .
	int const row_spacing = 0;
	int const col_spacing = 16;

	/// Extra space between frames and contents .
	SIZE const client_padding = {8, 8};
	SIZE const frame_padding = {2 , 2};

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

	char const * get_msg() const;

	size_t get_page_next_count() const;
	size_t get_page_prev_count() const;

private:
	CRITICAL_SECTION mutex;
	HANDLE queue_event;
	window::BackgroundDC offscreen;
	std::vector<font::EnumFontInfo> & fonts;
	std::vector<SIZE> font_sizes;
	std::vector<Job> jobs;
	char const * msg = nullptr;
	bool recalcFontSizes = true;

	void draw_fonts(size_t);

	void task();
};
