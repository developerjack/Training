#include <Windows.h>
#include <stdio.h>
#include "pentangle.h"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, PWSTR pCmdLine, int nCmdShow)
{
	wchar_t wszTitle[] = L"Basic Drawing";

	WNDCLASS wcls;
	ZeroMemory(&wcls, sizeof(WNDCLASS));
	wcls.style = CS_HREDRAW | CS_VREDRAW;
	wcls.lpfnWndProc = WindowProc;
	wcls.hInstance = hInstance;
	wcls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcls.hIcon = LoadIcon(NULL, IDI_SHIELD);
	wcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcls.lpszClassName = wszTitle;


	if (!RegisterClass(&wcls)) {
		MessageBox(NULL, L"Register class failed", L"Error", MB_OK);
		return GetLastError();
	}

	HWND hWndMain = CreateWindow(wszTitle, wszTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 600, NULL, NULL, hInstance, NULL);
	if (!hWndMain) {
		MessageBox(NULL, L"Create window failed", L"Error", MB_OK);
		return GetLastError();
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HPEN hPen = NULL, hPen2 = NULL;
	static HBRUSH hBrush = NULL;

	switch (uMsg) {
		case WM_CREATE:
		{
			hPen = CreatePen(PS_DOT, 1, RGB(0xff, 0, 0));
			hPen2 = CreatePen(PS_SOLID, 2, RGB(0xff, 0xcc, 0x44));
			hBrush = CreateSolidBrush(RGB(0xff, 0xee, 0xee));

			SetTimer(hWnd, 1, 100, NULL);

			ShowWindow(hWnd, SW_SHOW);
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT rc;
			GetClientRect(hWnd, &rc);

			// ��������ϵ
			SetMapMode(hdc, MM_ANISOTROPIC);
			SetViewportOrgEx(hdc, rc.right / 2, rc.bottom / 2, NULL);
			SetWindowExtEx(hdc, 1, -1, NULL);

			// Pen & Brush
			HGDIOBJ hOldPen = SelectObject(hdc, hPen);
			HGDIOBJ hOldBrush = SelectObject(hdc, hBrush);

			Ellipse(hdc, -200, 200, 200, -200);
			// x
			MoveToEx(hdc, -(rc.right / 2 - 50), 0, NULL);
			LineTo(hdc, rc.right / 2 - 50, 0);
			// y
			MoveToEx(hdc, 0, -(rc.bottom / 2 - 50), NULL);
			LineTo(hdc, 0, rc.bottom / 2 - 50);

			SetTextAlign(hdc, TA_CENTER);
			SetBkMode(hdc, TRANSPARENT);
			TextOut(hdc, 0, 0, L"O", 1);
			TextOut(hdc, rc.right / 2 - 50, 0, L"x+", 2);
			SetTextAlign(hdc, TA_LEFT);
			TextOut(hdc, 4, rc.bottom / 2 - 50, L"y+", 2);

			// �����
			DWORD dwCount = GetTickCount();
			SelectObject(hdc, hPen2);
			Pentangle(hdc, 0, 0, 200, dwCount * 3.14f / 30000.0f);
			Pentangle(hdc, 0, 0, 50, dwCount * 3.14f / 5000.0f);

			EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_TIMER:
		{
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
		case WM_DESTROY:
		{
			KillTimer(hWnd, 1);

			DeleteObject(hPen);
			DeleteObject(hBrush);

			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}