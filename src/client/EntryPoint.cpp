#include "pch.h"
#include "Application.h"

/*
==============
Link Library
==============
*/

#pragma comment(lib, "fmod_vc.lib")

extern "C" { __declspec(dllexport) AkU32 NvOptimusEnablement = 0x00000001; }

//////////////////////////////////////////////////////////////////////////////////////////////////////
// D3D12 Agility SDK Runtime

extern "C" { __declspec(dllexport) extern const AkU32 D3D12SDKVersion = 614; }

#if defined(_M_ARM64EC)
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\arm64\\"; }
#elif defined(_M_ARM64)
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\arm64\\"; }
#elif defined(_M_AMD64)
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\x64\\"; }
#elif defined(_M_IX86)
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\x86\\"; }
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hwnd, AkU32 msg, WPARAM wParam, LPARAM lParam);

/*
==============
Entry point
==============
*/

UApplication* g_pApplication;

int main(int argc, const char* argv[])
{
#ifdef _DEBUG
	int flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flags);
#endif

	const wchar_t wcClassName[] = L"Window Application";
	const wchar_t wcWindowName[] = L"Akatsuki Engine Prototype";

	WNDCLASS tWc = { };

	tWc.lpfnWndProc = WndProc;
	tWc.hInstance = ::GetModuleHandle(nullptr);
	tWc.lpszClassName = wcClassName;
	RegisterClass(&tWc);

	HWND hWnd = ::CreateWindowEx(0, wcClassName, wcWindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, tWc.hInstance, nullptr);

	::ShowWindow(hWnd, SW_SHOW);

	RECT tRt = {};
	::GetClientRect(hWnd, &tRt);
	AkU32 uScreenWidth = static_cast<AkU32>(tRt.right - tRt.left);
	AkU32 uScreenHeight = static_cast<AkU32>(tRt.bottom - tRt.top);

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

	UApplication* pApplication = new UApplication;
	if (!pApplication->InitApplication(hWnd, bEableDebugLayer, bEnableGBV))
	{
		__debugbreak();
	}
	g_pApplication = pApplication;

	// ::ShowCursor(AK_FALSE);

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
			pApplication->RunApplication();
		}
	}

	if (pApplication)
	{
		delete pApplication;
		pApplication = nullptr;
	}

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

	return static_cast<int>(tMsg.wParam);
}

LRESULT WndProc(HWND hWnd, AkU32 uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		AkI32 width = LOWORD(lParam);  // Macro to get the low-order word.
		AkI32 height = HIWORD(lParam); // Macro to get the high-order word.

		if (g_pApplication)
		{
			RECT tRect = {};
			::GetClientRect(hWnd, &tRect);
			AkU32 uScreenWidth = tRect.right - tRect.left;
			AkU32 uScreenHeight = tRect.bottom - tRect.top;

			g_pApplication->UpdateWindowSize(uScreenWidth, uScreenHeight);
		}
	}
	break;
	case WM_MOUSEMOVE:
	{
		// printf("%d %d\n", LOWORD(lParam), HIWORD(lParam));
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

