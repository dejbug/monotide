#include "lib_window.h"

int lib::window::run_main_loop(MSG & msg)
{
	while (1)
	{
		int res = GetMessage(&msg, NULL, 0, 0);
		if (0 == res) break; // WM_QUIT
		else if (-1 == res) return -1; // errors!
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
		LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;

	wc.lpszMenuName = NULL;

	wc.lpszClassName = nullptr;
	wc.lpfnWndProc = DefWindowProc;
}

ATOM lib::window::create_class(WNDCLASSEX & wc)
{
	return RegisterClassEx(&wc);
}

ATOM lib::window::create_class(LPCSTR name, WNDPROC callback)
{
	return create_class(name, callback,
		GetModuleHandle(nullptr));
}

ATOM lib::window::create_class(LPCSTR name, WNDPROC callback,
		HINSTANCE i)
{
	WNDCLASSEX wc;
	init_class(wc);

	wc.lpszClassName = name;
	wc.lpfnWndProc = callback;
	wc.hInstance = i;

	return RegisterClassEx(&wc);
}

HWND lib::window::create_frame(LPCSTR name, HINSTANCE i)
{
	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW|WS_EX_ACCEPTFILES|
			WS_EX_CONTEXTHELP|WS_EX_CONTROLPARENT,
		name,
		"",
		WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		i ? i : GetModuleHandle(nullptr),
		NULL
	);
}

HWND lib::window::create_child(HWND parent, UINT id, LPCSTR name,
		HINSTANCE i)
{
	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		name,
		"",
		WS_CHILD|WS_CLIPSIBLINGS|WS_TABSTOP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		parent,
		(HMENU)id,
		i ? i : GetModuleHandle(nullptr),
		NULL
	);
}

void lib::window::set_size(HWND handle, int width, int height)
{
	SetWindowPos(handle, NULL, 0, 0, width, height,
		SWP_NOZORDER|SWP_NOMOVE);
}

void lib::window::set_pos(HWND handle, int x, int y)
{
	SetWindowPos(handle, NULL, x, y, 0, 0,
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
	RECT r;
	GetClientRect(h, &r);
	s.cx = r.right;
	s.cy = r.bottom;
}
