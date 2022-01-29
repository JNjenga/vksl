#include <iostream>

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <tchar.h>

#pragma comment(lib, "Advapi32.lib")

#define ON_RGB RGB(r, g, b)
#define OFF_RGB RGB(128,128,128)

int width;
int height;

bool is_caps_on   = false;
bool is_scroll_on = false;
bool is_num_on    = false;

HKEY   hkey;
LPWSTR subkey       = TEXT("SOFTWARE\\Jnjenga\\VKSL");
LPWSTR x_option     = TEXT("x");
LPWSTR y_option     = TEXT("y");

LPWSTR active_color_option = TEXT("active_color");

// Active color
int r = 0;
int g = 0;
int b = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance,
		    HINSTANCE,
		    LPWSTR ,
		    int nCmdShow)
{
	// Create or open reg key
	LONG   lStatus;
	DWORD  disposition;
	DWORD  x_value, y_value, active_color;
	DWORD  type;
	DWORD  option_size = sizeof(int);

	lStatus = RegCreateKeyEx(HKEY_CURRENT_USER,
				 subkey, 
				 0,
				 NULL,
				 REG_OPTION_NON_VOLATILE,
				 KEY_WRITE,
				 NULL,
				 &hkey,
				 &disposition);

	if(lStatus != ERROR_SUCCESS)
	{
		printf("Some error occured in RegCreateKeyEx()\n");
		return -1;
	}

	// Get reg x and y values
	lStatus = RegGetValue(HKEY_CURRENT_USER,
			       subkey,
			       x_option,
			       RRF_RT_REG_DWORD,
			       &type,
			       &x_value,
			       &option_size);

	if(lStatus != ERROR_SUCCESS)
	{
		// TODO: Warning/error window
		printf("Some error occured when RegGetValueEx(): x_value\n");
		return -1;
	}

	lStatus = RegGetValue(HKEY_CURRENT_USER,
			       subkey,
			       y_option,
			       RRF_RT_REG_DWORD,
			       &type,
			       &y_value,
			       &option_size);

	if(lStatus != ERROR_SUCCESS)
	{
		// TODO: Warning/error window
		printf("Some error occured when RegGetValueEx(): y_value\n");
		return -1;
	}

	// Get active color value
	lStatus = RegGetValue(HKEY_CURRENT_USER,
			       subkey,
			       active_color_option,
			       RRF_RT_REG_DWORD,
			       &type,
			       &active_color,
			       &option_size);

	if(lStatus != ERROR_SUCCESS)
	{
		// TODO: Warning/error window
		printf("Some error occured when RegGetValueEx(): y_value\n");
		return -1;
	}

	// Set r,g,b values
	r = (active_color >> 24);
	g = (active_color << 8) >> 24;
	b = (active_color << 16) >> 24;

	MSG msg       = {};
	HWND hwnd     = {};
	WNDCLASSEX wc = {};

	wc.cbSize        = sizeof(wc);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.lpszClassName = TEXT("VirtualKeyboardStatusLightsClass");
	wc.hInstance     = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszMenuName  = NULL;
	wc.lpfnWndProc   = WndProc;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm       = LoadIcon(NULL, IDI_HAND);
	wc.hIcon         = LoadIcon(NULL, IDI_HAND);

	RegisterClassEx(&wc);
	hwnd = CreateWindowEx(0,
			wc.lpszClassName,
			TEXT("vksl v0.1"),
			     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			     x_value, y_value,
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
		int key_code = (int)msg.wParam;

			// Todo(James): Investigate if there is a better way of doing this
			// If removed windows will not register the key since it is registered as a hot key 
			// i.e Key will not be turned on/off
			UnregisterHotKey(NULL, key_code);
			keybd_event( (BYTE)key_code,
					0x45,
					KEYEVENTF_EXTENDEDKEY | 0,
					0 );
			keybd_event( (BYTE)key_code,
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
			width  = GetSystemMetrics(SM_CXSCREEN);
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
			// Update registry values
			RECT window_rect;
			if(GetWindowRect(hwnd, &window_rect) != 0)
			{
				LONG   lStatus;
				DWORD x = window_rect.left;
				DWORD y = window_rect.top;

				// Set reg values 
				lStatus = RegSetValueEx(hkey,
						        x_option,
							0,
							REG_DWORD,
							(const BYTE*)&x,
							sizeof(int));
				if(lStatus != ERROR_SUCCESS)
				{
					// TODO(James): Warning/Error window
					printf("Some error occured in RegSetValueEx(): x\n");
					return -1;
				}

				// Set reg values 
				lStatus = RegSetValueEx(hkey,
						        y_option,
							0,
							REG_DWORD,
							(const BYTE*)&y,
							sizeof(int));
				if(lStatus != ERROR_SUCCESS)
				{
					// TODO(James): Warning/Error window
					printf("Some error occured in RegSetValueEx(): y\n");
					return -1;
				}
			}
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
