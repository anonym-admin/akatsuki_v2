#include "pch.h"
#include "Window.h"

/*
==============
Link Library
==============
*/

#pragma comment(lib, "fmod_vc.lib")

extern "C" { __declspec(dllexport) AkU32 NvOptimusEnablement = 0x00000001; }

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

/*
==============
Entry point
==============
*/

int main(int argc, const char* argv[])
{
#ifdef _DEBUG
	AkI32 iFlags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(iFlags);
#endif

	GWindow->InitApplication();
	AkI32 iRetCode = GWindow->Run();

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

	return static_cast<int>(iRetCode);
}

