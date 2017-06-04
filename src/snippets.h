#pragma once
#include <windows.h>
#include "lib_font.h"

namespace snippets {

void calc_text_rect_1(OUT RECT &, HDC, lib::font::EnumFontInfo &, char const *, size_t);
void calc_text_rect_2(OUT RECT &, HDC, lib::font::EnumFontInfo &, char const *, size_t);

}
