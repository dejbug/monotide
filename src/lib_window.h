#ifndef MONOTIDE_LIB_WINDOW_H
#define MONOTIDE_LIB_WINDOW_H

#include <tchar.h>
#include <windows.h>

namespace lib {
namespace window {

struct BackgroundDC
{
	enum {
		E_OK, E_CCDC, E_CCBMP,
	} error;
	HDC parent;
	HDC handle;
	HBITMAP bmp;
	int w, h;

	BackgroundDC(HDC parent=nullptr);
	BackgroundDC(BackgroundDC const & other) = delete;
	BackgroundDC(BackgroundDC && other);
	virtual ~BackgroundDC();

	BackgroundDC& operator=(BackgroundDC const & other) = delete;
	BackgroundDC& operator=(BackgroundDC && other);

	void delete_bmp();
	void delete_dc();
	void resize(int w, int h);
	void resize();
	void flip();
	void clear(UINT id);
	void fill(COLORREF c);
};

int run_main_loop(MSG &, HWND main=nullptr, HACCEL haccel=nullptr);

void init_class(WNDCLASSEX & wc);
ATOM create_class(WNDCLASSEX & wc);
ATOM create_class(LPCTSTR clsname, WNDPROC);
ATOM create_class(LPCTSTR clsname, WNDPROC, HINSTANCE);

HWND create_frame(LPCTSTR clsname, HINSTANCE=nullptr);
HWND create_child(HWND parent, UINT id, LPCTSTR clsname, HINSTANCE=nullptr);

void set_size(HWND, int width, int height);
void set_pos(HWND, int x, int y);
void center_to_parent(HWND);

void get_inner_size(HWND, SIZE &);
void get_outer_size(HWND, SIZE &);

void quick_draw(HDC, int x, int y, LPCTSTR text, int text_len, int text_height, COLORREF);

void close_window(HWND h);

} // namespace window
} // namespace lib

#endif // !MONOTIDE_LIB_WINDOW_H
