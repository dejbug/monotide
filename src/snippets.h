#pragma once
#include <windows.h>
#include "lib_font.h"

namespace snippets {

void calc_text_rect_1(OUT RECT &, HDC, lib::font::EnumFontInfo &, char const *, size_t);
void calc_text_rect_2(OUT RECT &, HDC, char const *, size_t);


struct ScrollBar
{
	size_t count = 0;
	size_t index = 0;

	ScrollBar(HWND h, int nBar=SB_VERT);
	void show(bool on=true);
	void set_count(size_t count);
	bool scroll(int steps, bool update=true);
	void update();

private:
	HWND parent;
	int bar_id;
};


struct Worker
{
	enum {
		E_OK, E_CREATE, E_NO_RESTART, E_RESUME,
	} error;

	HANDLE handle;
	DWORD thread_id;
	bool running;

	Worker(bool auto_start=false);

	void start();
	void stop();
	bool wait(DWORD msec=INFINITE) const;

	virtual void task();
	virtual DWORD run();

private:
	static DWORD WINAPI main(LPVOID);
};

}
