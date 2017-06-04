#pragma once
#include <windows.h>
#include "lib_font.h"

namespace snippets {

void draw_font_label_1(HDC, IN OUT RECT &, lib::font::EnumFontInfo &);
void calc_text_rect(OUT RECT &, HDC, lib::font::EnumFontInfo &, char const *, size_t);

}
