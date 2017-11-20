#include "lib_window.h"
#include "macros.h"

lib::window::BackgroundDC::BackgroundDC(HDC parent)
		: error(E_OK), parent(parent), handle(nullptr), bmp(nullptr)
{
	if (!parent) return;

	handle = CreateCompatibleDC(parent);
	if (!handle) error = E_CCDC;
	else resize();
}

lib::window::BackgroundDC::BackgroundDC(lib::window::BackgroundDC && other)
		: error(other.error), parent(other.parent), handle(other.handle)
		, bmp(other.bmp), w(other.w), h(other.h)
{
	other.parent = nullptr;
	other.handle = nullptr;
	other.bmp = nullptr;
	other.w = 0;
	other.h = 0;
}

lib::window::BackgroundDC::~BackgroundDC()
{
	delete_bmp();
	delete_dc();
}

lib::window::BackgroundDC & lib::window::BackgroundDC::operator=(lib::window::BackgroundDC && other)
{
	parent = other.parent;
	handle = other.handle;
	bmp = other.bmp;
	w = other.w;
	h = other.h;

	other.parent = nullptr;
	other.handle = nullptr;
	other.bmp = nullptr;
	other.w = 0;
	other.h = 0;

	return *this;
}

void lib::window::BackgroundDC::delete_bmp()
{
	if (handle) SelectObject(handle, (HBITMAP)nullptr);
	if (bmp) { DeleteObject(bmp); bmp = nullptr; }
}

void lib::window::BackgroundDC::delete_dc()
{
	if (!handle) return;
	DeleteDC(handle);
	handle = nullptr;
}

void lib::window::BackgroundDC::resize(int w, int h)
{
	if (error) return;
	if (!handle) return;

	if (bmp) delete_bmp();

	bmp = CreateCompatibleBitmap(parent, w, h);
	if (!bmp)
	{
		error = E_CCBMP;
		return;
	}

	SelectObject(handle, (HBITMAP)bmp);

	this->w = w;
	this->h = h;
}

void lib::window::BackgroundDC::resize()
{
	if (error) return;
	RECT client_rect;
	GetClientRect(WindowFromDC(parent), &client_rect);
	resize(client_rect.right, client_rect.bottom);
}

void lib::window::BackgroundDC::flip()
{
	if (error) return;
	BitBlt(parent, 0, 0, w, h, handle, 0, 0, SRCCOPY);
}

void lib::window::BackgroundDC::clear(UINT id)
{
	if (error) return;
	RECT client_rect;
	GetClientRect(WindowFromDC(parent), &client_rect);
	FillRect(handle, &client_rect, (HBRUSH)(id+1));
}

void lib::window::BackgroundDC::fill(COLORREF c)
{
	if (error) return;

	HGDIOBJ old_pen = SelectObject(handle, GetStockObject(NULL_PEN));
	HGDIOBJ old_bru = SelectObject(handle, GetStockObject(DC_BRUSH));
	SetDCBrushColor(handle, c);

	RECT client_rect;
	GetClientRect(WindowFromDC(parent), &client_rect);
	Rectangle(handle, 0, 0, client_rect.right, client_rect.bottom);

	SelectObject(handle, old_pen);
	SelectObject(handle, old_bru);
}

int lib::window::run_main_loop(MSG & msg, HWND main, HACCEL haccel)
{
	while (1)
	{
		int const res = GetMessage(&msg, nullptr, 0, 0);
		if (0 == res) break; // WM_QUIT
		else if (-1 == res) return -1; // errors!
		if (main && haccel) TranslateAccelerator(main, haccel, &msg);
		if (main && IsDialogMessage(main, &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

void lib::window::init_class(WNDCLASSEX & wc)
{
	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	wc.style = CS_DBLCLKS | CS_HREDRAW |
		CS_VREDRAW | CS_OWNDC;

	wc.hInstance = GetModuleHandle(nullptr);
	wc.hIcon = wc.hIconSm =
		LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;

	wc.lpszMenuName = nullptr;

	wc.lpszClassName = nullptr;
	wc.lpfnWndProc = DefWindowProc;
}

ATOM lib::window::create_class(WNDCLASSEX & wc)
{
	return RegisterClassEx(&wc);
}

ATOM lib::window::create_class(LPCTSTR name, WNDPROC callback)
{
	return create_class(name, callback, GetModuleHandle(nullptr));
}

ATOM lib::window::create_class(LPCTSTR name, WNDPROC callback,
		HINSTANCE i)
{
	WNDCLASSEX wc;
	init_class(wc);

	wc.lpszClassName = name;
	wc.lpfnWndProc = callback;
	wc.hInstance = i;

	return RegisterClassEx(&wc);
}

HWND lib::window::create_frame(LPCTSTR name, HINSTANCE i)
{
	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW|WS_EX_ACCEPTFILES|
			WS_EX_CONTEXTHELP|WS_EX_CONTROLPARENT,
		name,
		_T(""),
		WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		i ? i : GetModuleHandle(nullptr),
		nullptr
	);
}

HWND lib::window::create_child(HWND parent, UINT id, LPCTSTR name,
		HINSTANCE i)
{
	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		name,
		_T(""),
		WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		parent,
		(HMENU)id,
		i ? i : GetModuleHandle(nullptr),
		nullptr
	);
}

void lib::window::set_size(HWND handle, int width, int height)
{
	SetWindowPos(handle, nullptr, 0, 0, width, height,
		SWP_NOZORDER|SWP_NOMOVE);
}

void lib::window::set_pos(HWND handle, int x, int y)
{
	SetWindowPos(handle, nullptr, x, y, 0, 0,
		SWP_NOZORDER|SWP_NOSIZE);
}

void lib::window::center_to_parent(HWND handle)
{
	RECT rp, rc;

	HWND parent = GetParent(handle);
	if (parent) GetWindowRect(parent, &rp);
	else rp = {0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN)};

	GetWindowRect(handle, &rc);

	int const rpw = (rp.right - rp.left);
	int const rph = (rp.bottom - rp.top);
	int const rcw = (rc.right - rc.left);
	int const rch = (rc.bottom - rc.top);

	int const dx = (rpw - rcw) >> 1;
	int const dy = (rph - rch) >> 1;

	set_pos(handle, rp.left + dx, rp.top + dy);
}

void lib::window::get_outer_size(HWND h, SIZE & s)
{
	RECT r;
	GetWindowRect(h, &r);
	s.cx = r.right - r.left;
	s.cy = r.bottom - r.top;
}

void lib::window::get_inner_size(HWND h, SIZE & s)
{
	// PRINT_VAR((UINT) h, "%08x");

	RECT r;
	GetClientRect(h, &r);
	// PRINT_VAR(r.right, "%ld");
	// PRINT_VAR(r.bottom, "%ld");
	s.cx = r.right - r.left;
	s.cy = r.bottom - r.top;
}

void lib::window::quick_draw(HDC dc, int x, int y, LPCTSTR text,
		int text_len, int text_height, COLORREF text_color)
{
	if (text_len < 0) text_len = _tcslen(text);
	HFONT hf = CreateFont(text_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, nullptr);

	SaveDC(dc);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, text_color);
	SelectObject(dc, hf);
	TextOut(dc, x, y, text, text_len);
	RestoreDC(dc, -1);

	DeleteObject(hf);
}

void lib::window::close_window(HWND h)
{
	SendMessage(h, WM_CLOSE, 0, 0);
}
