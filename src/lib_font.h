#ifndef MONOTIDE_LIB_FONT_H
#define MONOTIDE_LIB_FONT_H

#include <tchar.h>
#include <windows.h>
#include <vector>

namespace lib {
namespace font {

struct EnumFontInfo
{
	ENUMLOGFONTEX elfe;
	NEWTEXTMETRICEX ntme;
	int fontType = 0;

	EnumFontInfo(ENUMLOGFONTEX elfe, NEWTEXTMETRICEX ntme, int fontType);
};

struct EnumFontInfoLoader
{
	HDC dc = nullptr;
	HFONT handle = nullptr;
	HGDIOBJ old = nullptr;

	EnumFontInfoLoader(HDC, EnumFontInfo &);
	virtual ~EnumFontInfoLoader();

	EnumFontInfoLoader(EnumFontInfoLoader const &) = delete;
	EnumFontInfoLoader(EnumFontInfoLoader &&) = delete;

	void detach();
};

void list_fonts(OUT std::vector<EnumFontInfo> &,
		BYTE charSet=DEFAULT_CHARSET, bool onlyTrueType=false,
		HDC=nullptr, size_t maxCount=0xffffffff);

void sort_fonts(OUT std::vector<EnumFontInfo> &);

void print_font_info(EnumFontInfo &);

void draw_font_label(HDC, RECT &, EnumFontInfo &);

int get_sysfont_height();

}
}

#endif // !MONOTIDE_LIB_FONT_H
