/*
	Mostly discarded code, kept for reference and more readily
	available documentation of my thought process than git logs .
*/
#include "snippets.h"
#include "macros.h"
#include "lib_window.h"

#include <stdio.h>
#include <math.h>

void snippets::calc_text_rect_2(RECT & rc, HDC dc,
		char const * text, size_t text_len)
{
	SIZE text_size;
	GetTextExtentPoint32(dc, text, text_len, &text_size);
	rc.right = rc.left + text_size.cx;
	rc.bottom = rc.top + text_size.cy;
}

void snippets::calc_text_rect_1(RECT & rc, HDC dc,
		lib::font::EnumFontInfo & fi,
		char const * text, size_t text_len)
{
	lib::font::EnumFontInfoLoader efil(dc, fi);

	SIZE text_size;
	GetTextExtentPoint32(dc, text, text_len, &text_size);
	rc.right = rc.left + text_size.cx;
	rc.bottom = rc.top + text_size.cy;
}

snippets::ScrollBar::ScrollBar(HWND h, int nBar)
		: parent(h), bar_id(nBar)
{
}

void snippets::ScrollBar::show(bool on)
{
	ShowScrollBar(parent, bar_id, on ? TRUE : FALSE);
}

void snippets::ScrollBar::set_count(size_t _count)
{
	count = _count ? _count - 1 : 0;
	SetScrollRange(parent, bar_id, 0, count, FALSE);
	show(count > 0);
}

bool snippets::ScrollBar::scroll(int steps, bool update)
{
	if (count == 0) return false;

	size_t temp = index;

	if (steps >= 0)
	{
		if (count - index <= (size_t) steps)
		{
			temp = count;
		}
		else if (index + steps <= count)
		{
			temp += steps;
		}
	}
	else
	{
		if (index <= (size_t) -steps)
		{
			temp = 0;
		}
		else if (index > (size_t) -steps)
		{
			temp += steps;
		}
	}

	if (temp == index) return false;
	index = temp;

	if (update) this->update();

	return true;
}

void snippets::ScrollBar::update()
{
	SetScrollPos(parent, bar_id, index, TRUE);
}

snippets::RowIndexDrawer::RowIndexDrawer()
		: format{"%3d"}, buffer{0}
{
	HDC dc = GetDC(nullptr);
	SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetrics(dc, &tm);
	ReleaseDC(nullptr, dc);
}

int snippets::RowIndexDrawer::get_height(float scale) const
{
	return int(tm.tmHeight * scale);
}

void snippets::RowIndexDrawer::set_digits(unsigned count)
{
	_snprintf(format, sizeof(format),
		count >= 2 ? "%%%dd" : "%%d", count);
	// PRINT_VAR(format, "%s");
}

void snippets::RowIndexDrawer::set_digits_from_max_index(size_t index)
{
	set_digits(floor(log10(index ? index : 1) + 1));
}

void snippets::RowIndexDrawer::draw(HDC dc, RECT & rc,
		int index, char const * format)
{
	if (!format || !*format) format = this->format;
	_snprintf(buffer, sizeof(buffer), format, index);

	SetBkMode(dc, TRANSPARENT);
	HGDIOBJ old_font = SelectObject(dc, GetStockObject(ANSI_FIXED_FONT));
	text_draw_2(dc, rc, buffer, strlen(buffer));
	SelectObject(dc, old_font);
	SetBkMode(dc, OPAQUE);
}

void snippets::text_draw_1(HDC dc, RECT & rc,
		char const * text, size_t text_len)
{
	RECT tr = {rc.left, rc.top, 0, 0};
	calc_text_rect_2(tr, dc, text, text_len);

	UINT const ta_old = SetTextAlign(dc, TA_UPDATECP);

	MoveToEx(dc, rc.left, rc.top, nullptr);
	ExtTextOut(dc, rc.left, rc.top,
		0, nullptr, text, text_len, nullptr);

	MoveToEx(dc, rc.left, rc.top, (LPPOINT) &rc.right);
	rc.bottom = tr.bottom;

	if (GDI_ERROR != ta_old)
		SetTextAlign(dc, ta_old);
}

void snippets::text_draw_2(HDC dc, RECT & rc,
		char const * text, size_t text_len)
{
	calc_text_rect_2(rc, dc, text, text_len);
	TextOut(dc, rc.left, rc.top, text, text_len);
}
