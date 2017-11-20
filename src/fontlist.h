#ifndef MONOTIDE_FONTLIST_H
#define MONOTIDE_FONTLIST_H

#include <tchar.h>
#include <windows.h>

#define WC_FONTLIST					_T("dejbug.de/FontList")
#define WS_FONTLIST_DOUBLEBUFFER	(0x0001)
#define WM_FONTLIST_BASE			(WM_USER+2)

extern "C" {

void WINAPI RegisterFontList(void);
void WINAPI UnregisterFontList(void);

} // extern "C"

#endif // !MONOTIDE_FONTLIST_H
