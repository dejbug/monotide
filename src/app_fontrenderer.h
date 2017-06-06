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
	struct Job
	{
		size_t index;
		size_t & count_rendered;

		Job(size_t, size_t &);
	};

	FontRenderWorker();

	virtual ~FontRenderWorker();

	void setup(HWND, window::BackgroundDC &,
		std::vector<font::EnumFontInfo> &);

	void queue(size_t, size_t &);

	char const * get_msg() const;

private:
	CRITICAL_SECTION mutex;
	HANDLE queue_event;
	HWND hwnd = nullptr;
	window::BackgroundDC * offscreen = nullptr;
	std::vector<font::EnumFontInfo> * fonts;
	std::vector<Job> jobs;
	char const * msg = nullptr;

	void task();
};

void draw_fonts(HWND, HDC, std::vector<font::EnumFontInfo> &,
	size_t, size_t &);
