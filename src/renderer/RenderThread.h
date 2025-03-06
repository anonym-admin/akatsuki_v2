#pragma once

enum class RENDER_THREAD_EVENT_TYPE
{
	RENDER_THREAD_EVENT_TYPE_PROCESS,
	RENDER_THREAD_EVENT_TYPE_DESTROY,
	RENDER_THREAD_EVENT_TYPE_COUNT
};

struct RENDER_THREAD_DESC
{
	class FRenderer* pRenderer = nullptr;
	AkU32 uThreadIndex = 0;
	HANDLE hThread = nullptr;
	HANDLE hEventList[(AkU32)RENDER_THREAD_EVENT_TYPE::RENDER_THREAD_EVENT_TYPE_COUNT];
};

AkU32 RenderThreadByProcess(void* pParam);