#pragma once
#include <windows.h>

namespace lib {
namespace window {

	int run_main_loop(MSG &);

	void init_class(WNDCLASSEX & wc);
	ATOM create_class(WNDCLASSEX & wc);
	ATOM create_class(LPCSTR clsname, WNDPROC);
	ATOM create_class(LPCSTR clsname, WNDPROC, HINSTANCE);

	HWND create_frame(LPCSTR clsname, HINSTANCE=nullptr);
	HWND create_child(HWND parent, UINT id, LPCSTR clsname, HINSTANCE=nullptr);

	void set_size(HWND, int width, int height);
	void set_pos(HWND, int x, int y);
	void center_to_parent(HWND);

}
}
