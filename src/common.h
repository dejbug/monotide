#pragma once
#include <windows.h>

#define WM_FR_MESSAGE_UPDATE	(WM_USER + 1)

#define FR_DEBUG_SLOW_DRAW	 	500
// #define FR_DEBUG_TICK_DELAY		500
// #define FR_WAIT_AT_EXIT			1000

extern bool const draw_while_thumb_tracking;
extern bool const only_TTF;
