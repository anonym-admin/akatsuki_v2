#include "pch.h"
#include "RenderThread.h"
#include "Renderer.h"

AkU32 RenderThreadByProcess(void* pParam)
{
	RENDER_THREAD_DESC* pDesc = (RENDER_THREAD_DESC*)pParam;
	FRenderer* pRenderer = pDesc->pRenderer;
	AkU32 uThreadIndex = pDesc->uThreadIndex;
	const HANDLE* phEventList = pDesc->hEventList;
	AkBool bExit = AK_FALSE;

	while (AK_TRUE)
	{
		AkU32 uEventIndex = WaitForMultipleObjects((AkU32)RENDER_THREAD_EVENT_TYPE::RENDER_THREAD_EVENT_TYPE_COUNT, phEventList, AK_FALSE, INFINITE);

		switch ((RENDER_THREAD_EVENT_TYPE)uEventIndex)
		{
		case RENDER_THREAD_EVENT_TYPE::RENDER_THREAD_EVENT_TYPE_PROCESS:
			pRenderer->ProcessByThread(uThreadIndex);
			break;
		case RENDER_THREAD_EVENT_TYPE::RENDER_THREAD_EVENT_TYPE_DESTROY:
			bExit = AK_TRUE;
			break;
		default:
			__debugbreak();
			break;
		}

		if (bExit)
		{
			break;
		}
	}

	_endthreadex(998);
	return 997;
}
