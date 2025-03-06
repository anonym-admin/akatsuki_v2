#pragma once

/*
=========
Window
=========
*/

class Window
{
public:
	Window(const wchar_t* wcWindowName, const wchar_t* wcClassName);
	~Window();

	AkBool Initialize(const wchar_t* wcWindowName, const wchar_t* wcClassName);
	AkBool InitApplication();
	AkI32 Run();
	LRESULT CALLBACK MemWndProc(HWND hWnd, AkU32 uMsg, WPARAM wParam, LPARAM lParam);

private:
	void CleanUp();
};

