#include "stdafx.h"
#include "model.h"
#include "view.h"
#include "ViewMatrix.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrev, PWSTR pCmdLine, int nCmdShow)
{
	wchar_t wszTitle[] = L"Keyboard";

	WNDCLASS wcls;
	ZeroMemory(&wcls, sizeof(WNDCLASS));
	wcls.style = CS_HREDRAW | CS_VREDRAW;
	wcls.lpfnWndProc = WindowProc;
	wcls.hInstance = hInstance;
	wcls.hbrBackground = CreateSolidBrush(RGB(0x00, 0x00, 0x00));
	wcls.hIcon = LoadIcon(NULL, IDI_SHIELD);
	wcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcls.lpszClassName = wszTitle;


	if (!RegisterClass(&wcls)) {
		MessageBox(NULL, L"Register class failed", L"Error", MB_OK);
		return GetLastError();
	}

	HWND hWndMain = CreateWindow(wszTitle, L"键盘", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (!hWndMain) {
		MessageBox(NULL, L"Create window failed", L"Error", MB_OK);
		return GetLastError();
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteObject(wcls.hbrBackground);

	return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static Model model;
	static View view;
	static ViewMatrix vMatrix;
	static HDC hdcMem;
	static HBITMAP hBmp;
	static LARGE_INTEGER bigFreq, bigCount;
	static int nMaxFPS = 40;
	static BOOL bNoLimitFPS = TRUE;

	switch (uMsg) {
	case WM_CREATE:
	{
		// 初始化
		ModelInit(&model);
		ViewInit(&view, &model, hWnd);
		ViewMatrixInit(&vMatrix, &model, hWnd);

		// 最大可能的桌面尺寸
		int maxWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int maxHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

		// 双缓冲
		HDC hdc = GetDC(hWnd);
		hdcMem = CreateCompatibleDC(hdc);
		hBmp = CreateCompatibleBitmap(hdc, maxWidth, maxHeight);
		SelectObject(hdcMem, hBmp);
		ReleaseDC(hWnd, hdc);

		QueryPerformanceFrequency(&bigFreq);
		QueryPerformanceCounter(&bigCount);

		ShowWindow(hWnd, SW_SHOW);

		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT rect;
		HDC hdc;
		LARGE_INTEGER bigNow;

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);
		// 绘制在不可见的缓冲区上
		view.pAPI->OnPaint(&view, hdcMem);
		vMatrix.pAPI->OnPaint(&vMatrix, hdcMem);
		// 复制到可见的前台缓冲区
		BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);

		// FPS
		QueryPerformanceCounter(&bigNow);
		int fps = (int)(bigFreq.QuadPart / (bigNow.QuadPart - bigCount.QuadPart));
		bigCount = bigNow;
		view.pAPI->SetFPS(&view, fps, nMaxFPS, bNoLimitFPS);

		EndPaint(hWnd, &ps);

		// 立即重绘FPS机制
		if (model.pAPI->GetStringCount(&model) > 0) {
			if (bNoLimitFPS || fps <= nMaxFPS) {
				InvalidateRect(hWnd, NULL, FALSE);
			}
			else {
				PostMessage(hWnd, WM_USER, 0, 0);
			}
		}
		return 0;
	}
	case WM_USER:
	{
		LARGE_INTEGER bigNow;
		QueryPerformanceCounter(&bigNow);
		int fps = (int)(bigFreq.QuadPart / (bigNow.QuadPart - bigCount.QuadPart));
		if (bNoLimitFPS || fps <= nMaxFPS) {
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else {
			Sleep((fps - nMaxFPS) * 1000 / (fps*nMaxFPS));
			PostMessage(hWnd, WM_USER, 0, 0);
		}
		return 0;
	}
	case WM_CHAR:
	{
		model.pAPI->Put(&model, (wchar_t)wParam);
		InvalidateRect(hWnd, NULL, FALSE);

		return 0;
	}
	case WM_KEYDOWN:
	{
		if (GetKeyState(VK_CONTROL) & 0x8000) {
			COLORREF color = 0;
			// CTRL is pressed down
			switch (wParam)
			{
			case L'R': color = RGB(0xff, 0x00, 0x00); break;
			case L'G': color = RGB(0x00, 0xff, 0x00); break;
			case L'B': color = RGB(0x00, 0x00, 0xff); break;
			case VK_OEM_PLUS: nMaxFPS++; break;
			case VK_OEM_MINUS: nMaxFPS > 1 && nMaxFPS--; break;
			case VK_SPACE: bNoLimitFPS = !bNoLimitFPS; break;
			}

			if (color) {
				view.pAPI->ChangeColor(&view, color);
				vMatrix.pAPI->ChangeColor(&vMatrix, color);
				InvalidateRect(hWnd, NULL, FALSE);
			}

			return 0;
		}
		break;
	}
	case WM_SIZE:
	{
		RECT rc;
		rc.left = 0;
		rc.top = view.pAPI->GetHeight(&view) + 1;
		rc.right = LOWORD(lParam);
		rc.bottom = HIWORD(lParam);

		vMatrix.pAPI->SetRect(&vMatrix, &rc);

		return 0;
	}
	case WM_DESTROY:
	{
		model.pAPI->Clear(&model);
		view.pAPI->Close(&view);
		vMatrix.pAPI->Close(&vMatrix);

		DeleteDC(hdcMem);
		DeleteObject(hBmp);

		PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}