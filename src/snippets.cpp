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

/// draw_font_label_*()

/// -- These are intended to be used in the drawing logic of font selection
/// browsers which show a font preview . The preview is just the title of
/// the font drawn in that font .

/// -- The functions' numeric suffix is to be read as version number . It
/// reflects my difficulties with the attempt to have the function return
/// an accurate RECT in 'rc' of the drawn text's boundary . The RECT is
/// supposed to be used in later stages e.g. to draw a frame around the
/// label or to position some labels side-by-side if they fit inside a row .
