#pragma once
#include <windows.h>
#include <vector>

namespace lib {
namespace font {

struct EnumFontInfo
{
	ENUMLOGFONTEX elfe;
	NEWTEXTMETRICEX ntme;
	int FontType = 0;

	EnumFontInfo(ENUMLOGFONTEX elfe, NEWTEXTMETRICEX ntme, int FontType)
			: elfe(elfe), ntme(ntme), FontType(FontType)
	{
	}
};

void list_fonts(std::vector<EnumFontInfo> & out,
		BYTE charSet=DEFAULT_CHARSET, bool onlyTrueType=false,
		HDC hdc=nullptr, size_t maxCount=0xffffffff);

}
}
