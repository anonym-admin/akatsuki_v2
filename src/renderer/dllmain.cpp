#include "pch.h"
#include "Renderer.h"

// Directx lib
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

/*
===========
Main Entry
===========
*/

AkBool APIENTRY DllMain(HMODULE hModule,
    AkU32  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
#ifdef _DEBUG
        int flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
        _CrtSetDbgFlag(flags);
#endif
    }
    break;
    case DLL_THREAD_ATTACH:
    {

    }
    break;
    case DLL_THREAD_DETACH:
    {
    }
    break;
    case DLL_PROCESS_DETACH:
    {
#ifdef _DEBUG
        _ASSERT(_CrtCheckMemory());
#endif
    }
    break;
    }
    return TRUE;
}

void DLL_CreateInstance(void** ppModule)
{
    IRenderer** pRenderer = reinterpret_cast<IRenderer**>(ppModule);
    if (*pRenderer)
    {
        __debugbreak();
    }
    *pRenderer = new FRenderer;
}