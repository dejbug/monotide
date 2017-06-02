#include "lib_font.h"
#include <stdio.h>

int CALLBACK list_fonts_cb(
		LOGFONTA const * lpelf, TEXTMETRICA const * lpntme,
		long unsigned int FontType, LPARAM lParam)
{
	ENUMLOGFONTEX const * lpelfe = (ENUMLOGFONTEX const *) lpelf;
	unsigned & count = * (unsigned *) lParam;

	char const * ft = "?";
	switch(FontType)
	{
		case DEVICE_FONTTYPE: ft = "D"; break;
		case RASTER_FONTTYPE: ft = "R"; break;
		case TRUETYPE_FONTTYPE: ft = "T"; break;
	}

	printf("%4d : %s '%s'\n", count++, ft, lpelfe->elfFullName);
	return TRUE;
}

void lib::font::list_fonts(HDC hdc)
{
	unsigned count = 0;

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfCharSet = DEFAULT_CHARSET;
	EnumFontFamiliesEx(hdc, &lf, list_fonts_cb, (LPARAM) &count, 0);
}
