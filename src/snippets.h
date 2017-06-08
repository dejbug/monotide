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

struct RowIndexDrawer
{
	TEXTMETRIC tm;
	char buffer[32];

	RowIndexDrawer();
	int get_height(float scale=1.0) const;
	void draw(HDC, RECT &, int index, char const * format="%d");
};

void text_draw_1(HDC, RECT &, char const * text, size_t text_len);
void text_draw_2(HDC, RECT &, char const * text, size_t text_len);

}
