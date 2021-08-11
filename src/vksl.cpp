#include <windows.h>
#include <iostream>

#define ON_RGB RGB(255, 0, 0)
#define OFF_RGB RGB(128,128,128)

int width;
int height;

bool is_caps_on   = false;
bool is_scroll_on = false;
bool is_num_on    = false;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance,
		    HINSTANCE hPrevInstance,
		    PWSTR pCmdLine,
		    int nCmdShow)
{
	MSG msg = {};
	HWND hwnd = {};
	WNDCLASSW wc = {};

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.lpszClassName = L"VirtualKeyboardStatusLightsClass";
	wc.hInstance     = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszMenuName  = NULL;
	wc.lpfnWndProc   = WndProc;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassW(&wc);
	hwnd = CreateWindowW(wc.lpszClassName,
			     L"vksl v0.1",
			     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			     100, 100,
			     170, 100,
			     NULL, NULL,
			     hInstance,
			     NULL);  

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
	lStyle &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX );
	SetWindowLong(hwnd, GWL_STYLE, lStyle);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );


	RegisterHotKey(NULL, VK_CAPITAL, MOD_NOREPEAT, VK_CAPITAL);
	RegisterHotKey(NULL, VK_NUMLOCK, MOD_NOREPEAT, VK_NUMLOCK);
	RegisterHotKey(NULL, VK_SCROLL,  MOD_NOREPEAT, VK_SCROLL);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		DispatchMessage(&msg);
		if (msg.message == WM_HOTKEY)
		{
			auto key_code = msg.wParam;

			// Todo(James): Investigate if there is a better way of doing this
			// If removed windows will not register the key since it is registered as a hot key 
			// i.e Key will not be turned on/off
			UnregisterHotKey(NULL, key_code);
			keybd_event( key_code,
					0x45,
					KEYEVENTF_EXTENDEDKEY | 0,
					0 );
			keybd_event( key_code,
					0x45,
					KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
					0);

			RegisterHotKey(NULL, key_code, MOD_NOREPEAT, key_code);

			if ( key_code == VK_CAPITAL)
				is_caps_on = !is_caps_on;
			else if ( key_code == VK_NUMLOCK)
				is_num_on = !is_num_on;
			else if ( key_code == VK_SCROLL)
				is_scroll_on = !is_scroll_on;

			InvalidateRect(hwnd, 0, TRUE);
		}

	}

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, 
		WPARAM wParam, LPARAM lParam) {

	switch(msg)
	{
		case WM_CREATE:
		{
			width = GetSystemMetrics(SM_CXSCREEN);
			height = GetSystemMetrics(SM_CYSCREEN);

			// Initial state of keys lock
			if ((GetKeyState(VK_CAPITAL) & 0x0001)!=0)
				is_caps_on = true;

			if ((GetKeyState(VK_NUMLOCK) & 0x0001)!=0)
				is_num_on = true;

			if ((GetKeyState(VK_SCROLL) & 0x0001) != 0)
				is_scroll_on = true;

		} break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HPEN hPen = CreatePen(PS_NULL, 1, RGB(0, 0, 0));
			SelectObject(hdc, hPen);

			SelectObject(hdc, GetStockObject(DC_BRUSH));

			// Numlock
			SetDCBrushColor(hdc, is_num_on ? ON_RGB : OFF_RGB);
			Rectangle(hdc, 10, 10, 40, 40);

			// Caps
			SetDCBrushColor(hdc, is_caps_on ? ON_RGB : OFF_RGB);
			Rectangle(hdc, 60, 10, 90, 40);

			// Scroll lock
			SetDCBrushColor(hdc, is_scroll_on ? ON_RGB : OFF_RGB);
			Rectangle(hdc, 110, 10, 140, 40);

			HFONT hfont = CreateFontW(15, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0,
					0, 0, 0, 0, L"Arial");

			SelectObject(hdc, hfont);

			static wchar_t *caps_label = L"Caps";
			static wchar_t *num_label = L"Num";
			static wchar_t *scroll_label = L"Scroll";
			// static wchar_t *footer_label = L"v0.1";

			TextOutW(hdc, 10, 45,  num_label,  lstrlenW(num_label));
			TextOutW(hdc, 60, 45,  caps_label,  lstrlenW(caps_label));
			TextOutW(hdc, 110, 45,  scroll_label,  lstrlenW(scroll_label));

			// SetTextColor(hdc, 0x808080);
			// TextOutW(hdc, 65, 65,  footer_label,  lstrlenW(footer_label));

			DeleteObject(hPen);

			EndPaint(hwnd, &ps);
		} break;
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				SendMessage(hwnd, WM_CLOSE, 0, 0);
			}
		} break;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}
