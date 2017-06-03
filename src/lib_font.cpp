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
		case DEVICE_FONTTYPE: ft = "dev"; break;
		case RASTER_FONTTYPE: ft = "ras"; break;
		case TRUETYPE_FONTTYPE: ft = "ttf"; break;
	}

	printf("%4d | %s | %s | %s | %d | %s |\n", count++, ft,
		lpelfe->elfLogFont.lfFaceName, // lpelfe->elfFullName,
		lpelfe->elfStyle,
		lpelfe->elfLogFont.lfCharSet,
		lpelfe->elfScript);

	return TRUE;
}

void lib::font::list_fonts(HDC hdc)
{
	unsigned count = 0;

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	// lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfCharSet = ANSI_CHARSET;
	EnumFontFamiliesEx(hdc, &lf, list_fonts_cb, (LPARAM) &count, 0);
}
