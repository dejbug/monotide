#include "lib_font.h"
#include <stdio.h>

lib::font::EnumFontInfo::EnumFontInfo(
		ENUMLOGFONTEX elfe, NEWTEXTMETRICEX ntme, int FontType)
		: elfe(elfe), ntme(ntme), FontType(FontType)
{
}

lib::font::EnumFontInfoLoader::EnumFontInfoLoader(
		HDC dc, lib::font::EnumFontInfo & efi)
		: dc(dc)
{
	handle = CreateFontIndirect(&efi.elfe.elfLogFont);
	old = SelectObject(dc, handle);
}

lib::font::EnumFontInfoLoader::~EnumFontInfoLoader()
{
	if(handle) {
		SelectObject(dc, old);
		DeleteObject(handle);
	}
}

void lib::font::EnumFontInfoLoader::detach()
{
	handle = nullptr;
}

struct ListFontsContext
{
	size_t index = 0;
	std::vector<lib::font::EnumFontInfo> * out = nullptr;
	size_t maxCount = 0xffffffff;
	bool onlyTrueType = false;
};

int CALLBACK list_fonts_cb(
		LOGFONTA const * lpelf, TEXTMETRICA const * lptm,
		long unsigned int FontType, LPARAM lParam)
{
	ENUMLOGFONTEX const * lpelfe = (ENUMLOGFONTEX const *) lpelf;
	NEWTEXTMETRICEX const * lpntme = (NEWTEXTMETRICEX const *) lptm;
	ListFontsContext & ctx = * (ListFontsContext *) lParam;

	if (!ctx.onlyTrueType || TRUETYPE_FONTTYPE == FontType)
		ctx.out->push_back(
			lib::font::EnumFontInfo(*lpelfe, *lpntme, FontType));

	++ctx.index;
	return ctx.out->size() < ctx.maxCount ? TRUE : FALSE;
}

void lib::font::list_fonts(std::vector<lib::font::EnumFontInfo> & out,
	BYTE charSet, bool onlyTrueType, HDC hdc, size_t maxCount)
{
	ListFontsContext ctx;
	ctx.out = &out;
	ctx.maxCount = maxCount;
	ctx.onlyTrueType = onlyTrueType;

	if(!hdc) hdc = GetDC(nullptr);

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfCharSet = charSet;
	EnumFontFamiliesEx(hdc, &lf, list_fonts_cb, (LPARAM) &ctx, 0);
}
