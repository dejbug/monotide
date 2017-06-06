/*
	Mostly discarded code, kept for reference and more readily
	available documentation of my thought process than git logs .
*/
#include "snippets.h"
#include "macros.h"
#include "lib_window.h"

#include <stdio.h>

void snippets::calc_text_rect_2(RECT & rc, HDC dc,
		char const * text, size_t text_len)
{
	// TEXTMETRIC tm;
	// GetTextMetrics(dc, &tm);

	// GetCharWidth32(dc, 0, 256-1, char_widths);

	// long offsets_sum = 0;
	// for (size_t i=0; i<text_len; ++i)
	// {
	// 	// GetCharWidth32(dc, text[i], text[i], &text_offsets[i]);
	// 	// text_offsets[i] = fi.elfe.elfLogFont.lfWidth;
	// 	// text_offsets[i] = tm.tmMaxCharWidth;
	// 	// if (text[i] >= 0 && text[i] < 256)
	// 	// 	text_offsets[i] = char_widths[(unsigned char) text[i]];
	// 	// else
	// 	// 	text_offsets[i] = tm.tmMaxCharWidth;
	// 	text_offsets[i] = tm.tmAveCharWidth;
	// 	offsets_sum += text_offsets[i];
	// }

	SIZE text_size;
	GetTextExtentPoint32(dc, text, text_len, &text_size);
	// text_size.cx = offsets_sum;

	// RECT tr;
	// int const text_height = DrawTextEx(dc, (char *) text, text_len, &tr,
	// 	DT_CALCRECT|DT_NOCLIP|DT_SINGLELINE, nullptr);
	// if (text_height > 0)
	// {
	// 	text_size.cx = tr.right - tr.left;
	// 	text_size.cy = text_height;
	// }

	rc.right = rc.left + text_size.cx;
	rc.bottom = rc.top + text_size.cy;

	// DrawTextEx(dc, (char *) text, text_len, (RECT *) &rc,
	// 	DT_CALCRECT|DT_NOCLIP|DT_SINGLELINE, nullptr);

}

void snippets::calc_text_rect_1(RECT & rc, HDC dc,
		lib::font::EnumFontInfo & fi,
		char const * text, size_t text_len)
{
	lib::font::EnumFontInfoLoader efil(dc, fi);

	// TEXTMETRIC tm;
	// GetTextMetrics(dc, &tm);

	// GetCharWidth32(dc, 0, 256-1, char_widths);

	// long offsets_sum = 0;
	// for (size_t i=0; i<text_len; ++i)
	// {
	// 	// GetCharWidth32(dc, text[i], text[i], &text_offsets[i]);
	// 	// text_offsets[i] = fi.elfe.elfLogFont.lfWidth;
	// 	// text_offsets[i] = tm.tmMaxCharWidth;
	// 	// if (text[i] >= 0 && text[i] < 256)
	// 	// 	text_offsets[i] = char_widths[(unsigned char) text[i]];
	// 	// else
	// 	// 	text_offsets[i] = tm.tmMaxCharWidth;
	// 	text_offsets[i] = tm.tmAveCharWidth;
	// 	offsets_sum += text_offsets[i];
	// }

	SIZE text_size;
	GetTextExtentPoint32(dc, text, text_len, &text_size);
	// text_size.cx = offsets_sum;

	// RECT tr;
	// int const text_height = DrawTextEx(dc, (char *) text, text_len, &tr,
	// 	DT_CALCRECT|DT_NOCLIP|DT_SINGLELINE, nullptr);
	// if (text_height > 0)
	// {
	// 	text_size.cx = tr.right - tr.left;
	// 	text_size.cy = text_height;
	// }

	rc.right = rc.left + text_size.cx;
	rc.bottom = rc.top + text_size.cy;

	// DrawTextEx(dc, (char *) text, text_len, (RECT *) &rc,
	// 	DT_CALCRECT|DT_NOCLIP|DT_SINGLELINE, nullptr);

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

snippets::Worker::Worker(bool auto_start)
		: error(E_OK)
		, handle(nullptr)
		, running(false)
{
	handle = CreateThread(nullptr, 0, main, (LPVOID) this,
		auto_start ? 0 : CREATE_SUSPENDED , &thread_id);

	if (!handle) error = E_CREATE;
	else running = auto_start;
}

void snippets::Worker::start()
{
	if (error) return;
	if (running) return;
	if (!handle)
	{
		error = E_NO_RESTART;
		return;
	}

	if ((DWORD) -1 == ResumeThread(handle))
	{
		error = E_RESUME;
		return;
	}

	running = true;
}

void snippets::Worker::stop()
{
	running = false;
}

bool snippets::Worker::wait(DWORD msec) const
{
	if (error) return false;
	if (!handle) return false;
	return WAIT_OBJECT_0 == WaitForSingleObject(handle, msec);
}

void snippets::Worker::task()
{
	Sleep(1000);
	printf(" [ worker %08x ] tick\n", (size_t) this);
	running = false;
}

DWORD snippets::Worker::run()
{
	printf(" [ Worker %08x ] enter\n", (size_t) this);

	while (running)
	{
		task();
	}

	printf(" [ Worker %08x ] leave\n", (size_t) this);

	return 0;
}

DWORD WINAPI snippets::Worker::main(LPVOID param)
{
	Worker * worker = (Worker *) param;
	return worker->run();
}
