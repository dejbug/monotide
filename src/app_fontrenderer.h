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

	struct Job
	{
		size_t index;
		size_t & count_rendered;

		Job(size_t, size_t &);
	};

	FontRenderWorker(std::vector<font::EnumFontInfo> &);

	virtual ~FontRenderWorker();

	void on_parent_resize();

	void queue(size_t, size_t &);

	char const * get_msg() const;

private:
	CRITICAL_SECTION mutex;
	HANDLE queue_event;
	window::BackgroundDC offscreen;
	std::vector<font::EnumFontInfo> & fonts;
	std::vector<Job> jobs;
	char const * msg = nullptr;

	void task();
};

void draw_fonts(HWND, HDC, std::vector<font::EnumFontInfo> &,
	size_t, size_t &);
