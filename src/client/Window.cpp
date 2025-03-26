#include "pch.h"
#include "Window.h"
#include "Application.h"

AkBool GFullScreen = AK_TRUE;

LRESULT CALLBACK WndProc(HWND hwnd, AkU32 msg, WPARAM wParam, LPARAM lParam);

/*
=========
Window
=========
*/

Window::Window(const wchar_t* wcWindowName, const wchar_t* wcClassName)
{
	if (!Initialize(wcWindowName, wcClassName))
	{
		__debugbreak();
	}
}

Window::~Window()
{
	CleanUp();
}

AkBool Window::Initialize(const wchar_t* wcWindowName, const wchar_t* wcClassName)
{
	WNDCLASS tWc = { };

	tWc.lpfnWndProc = WndProc;
	tWc.hInstance = ::GetModuleHandle(nullptr);
	tWc.lpszClassName = wcClassName;
	RegisterClass(&tWc);

	AkU32 uScreenWidth = CW_USEDEFAULT;
	AkU32 uScreenHeight = CW_USEDEFAULT;

	AkI32 iPosX = CW_USEDEFAULT;
	AkI32 iPosY = CW_USEDEFAULT;

	if (GFullScreen)
	{
		iPosX = 0;
		iPosY = 0;

		uScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		uScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		DEVMODE dmScreenSetting = {};
		dmScreenSetting.dmSize = sizeof(dmScreenSetting);
		dmScreenSetting.dmPelsWidth = uScreenWidth;
		dmScreenSetting.dmPelsHeight = uScreenHeight;
		dmScreenSetting.dmBitsPerPel = 32;
		dmScreenSetting.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSetting, CDS_FULLSCREEN);
	}

	HWND hWnd = ::CreateWindowEx(0, wcClassName, wcWindowName, WS_OVERLAPPEDWINDOW, iPosX, iPosY, uScreenWidth, uScreenHeight, nullptr, nullptr, tWc.hInstance, nullptr);

	::ShowWindow(hWnd, SW_SHOW);

	GhWnd = hWnd;

	return AK_TRUE;
}

AkBool Window::InitApplication()
{
#ifdef _DEBUG
	AkBool bEableDebugLayer = AK_FALSE;
	AkBool bEnableGBV = AK_FALSE;
	AkI32 iInput = 0;
	wprintf_s(L"[Select Mode]\n");
	wprintf_s(L"1. Enable debug layer and GPU based validation.\n");
	wprintf_s(L"2. Not debug option.\n");
	scanf_s("%d", &iInput);
	if (iInput == 1)
	{
		bEableDebugLayer = AK_TRUE;
		bEnableGBV = AK_TRUE;
	}
#else
	AkBool bEableDebugLayer = AK_FALSE;
	AkBool bEnableGBV = AK_FALSE;
#endif

	Application* pApplication = new Application;
	if (!pApplication->InitApplication(bEableDebugLayer, bEnableGBV))
	{
		__debugbreak();
	}

	GApp = pApplication;

	return AK_TRUE;
}

AkI32 Window::Run()
{
	MSG tMsg = { };
	while (true)
	{
		if (PeekMessage(&tMsg, nullptr, 0, 0, PM_REMOVE))
		{
			if (tMsg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&tMsg);
			DispatchMessage(&tMsg);
		}
		else
		{
			GApp->RunApplication();
		}
	}

	return (AkI32)tMsg.wParam;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::MemWndProc(HWND hWnd, AkU32 uMsg, WPARAM wParam, LPARAM lParam)
{
	if (GImGui && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return AK_TRUE;

	switch (uMsg)
	{
	case WM_SIZE:
	{
		AkI32 width = LOWORD(lParam);  // Macro to get the low-order word.
		AkI32 height = HIWORD(lParam); // Macro to get the high-order word.

		if (GApp)
		{
			RECT tRect = {};
			::GetClientRect(hWnd, &tRect);
			AkU32 uScreenWidth = tRect.right - tRect.left;
			AkU32 uScreenHeight = tRect.bottom - tRect.top;

			GApp->UpdateWindowSize(uScreenWidth, uScreenHeight);
		}
	}
	break;
	case WM_MOUSEMOVE:
	{
	}
	break;
	case WM_DESTROY:
	{
		::PostQuitMessage(999);
	}
	break;
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::CleanUp()
{
	if (GApp)
	{
		delete GApp;
		GApp = nullptr;
	}
}

LRESULT WndProc(HWND hWnd, AkU32 uMsg, WPARAM wParam, LPARAM lParam)
{
	return GWindow->MemWndProc(hWnd, uMsg, wParam, lParam);
}

