#include "pch.h"
#include "InputUI.h"
#include "GameInput.h"
#include "Application.h"

/*
==========
Input UI
==========
*/

UInputUI::UInputUI()
{
}

UInputUI::~UInputUI()
{
	CleanUp();
}

AkBool UInputUI::Initialize(UApplication* pApp, AkU32 uTextTextureWidth, AkU32 uTextTextureHeight, const wchar_t* wcFontFamilyName, AkF32 fFontSize)
{
	if (!UTextUI::Initialize(pApp, uTextTextureWidth, uTextTextureHeight, wcFontFamilyName, fFontSize))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Create the Chatting Thread.
	AkU32 uThreadID = 0;
	_tChatParam.pApp = pApp;
	_tChatParam.pUI = this;
	_tChatParam.hThread = (HANDLE)_beginthreadex(nullptr, 0, ProcessChatting, &_tChatParam, 0, &uThreadID);
	for (AkU32 i = 0; i < (AkU32)CHAT_THREAD_EVENT_TYPE::CHAT_THREAD_EVENT_COUNT; i++)
	{
		_tChatParam.hEvent[i] = (HANDLE)CreateEvent(nullptr, AK_FALSE, AK_FALSE, nullptr);
	}

	return AK_TRUE;
}

void UInputUI::Update(const AkF32 fDeltaTime)
{
	UUI::Update(fDeltaTime);

	SetEvent(_tChatParam.hEvent[(AkU32)CHAT_THREAD_EVENT_TYPE::CHAT_THREAD_EVENT_START]);
}

void UInputUI::CleanUp()
{
	SetEvent(_tChatParam.hEvent[(AkU32)CHAT_THREAD_EVENT_TYPE::CHAT_THREAD_EVNET_END]);

	WaitForSingleObject(_tChatParam.hThread, INFINITE);
	for (AkU32 i = 0; i < (AkU32)CHAT_THREAD_EVENT_TYPE::CHAT_THREAD_EVENT_COUNT; i++)
	{
		CloseHandle(_tChatParam.hEvent[i]);
		_tChatParam.hEvent[i] = nullptr;
	}
	CloseHandle(_tChatParam.hThread);
	_tChatParam.hThread = nullptr;
}


/*
======================
Thread Function
======================
*/

AkU32 ProcessChatting(void* pChatParam)
{
	static wchar_t wcText[256] = {};

	CHAT_THREAD_PARAM* pParam = (CHAT_THREAD_PARAM*)pChatParam;
	UApplication* pApp = (UApplication*)pParam->pApp;
	UGameInput* pGameInput = pApp->GetGameInput();
	UInputUI* pDynamicText = (UInputUI*)pParam->pUI;
	HANDLE* pEvent = pParam->hEvent;
	AkBool bEndFlag = AK_FALSE;

	if (!pGameInput)
	{
		__debugbreak();
	}

	while (AK_TRUE)
	{
		AkU32 uIndex = WaitForMultipleObjects((AkU32)CHAT_THREAD_EVENT_TYPE::CHAT_THREAD_EVENT_COUNT, pEvent, AK_FALSE, INFINITE);

		switch ((CHAT_THREAD_EVENT_TYPE)uIndex)
		{
		case CHAT_THREAD_EVENT_TYPE::CHAT_THREAD_EVENT_START:

			if (pDynamicText->IsMouseOn()) // 해당 함수가 적용되기 위해선 UI Manager 에서 관리되어야 함.
			{
				ProcessChatInput(pGameInput, wcText);
				pDynamicText->WriteText(wcText);
			}

			break;
		case CHAT_THREAD_EVENT_TYPE::CHAT_THREAD_EVNET_END:

			bEndFlag = AK_TRUE;
			_endthreadex(123);

			break;
		}

		if (bEndFlag)
		{
			break;
		}
	}

	return 125;
}

void ProcessChatInput(class UGameInput* pGameInput, wchar_t* wcBuf)
{
	static AkI32 i = 0;
	static wchar_t wcPrevBuf[256] = {};

	wchar_t wc = 0;

	if (pGameInput->KeyFirstDown(KEY_INPUT_A))
	{
		wc = 'A';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_B))
	{
		wc = 'B';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_C))
	{
		wc = 'C';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_D))
	{
		wc = 'D';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_E))
	{
		wc = 'E';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_F))
	{
		wc = 'F';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_G))
	{
		wc = 'G';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_H))
	{
		wc = 'H';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_I))
	{
		wc = 'I';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_J))
	{
		wc = 'J';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_K))
	{
		wc = 'K';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_L))
	{
		wc = 'L';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_M))
	{
		wc = 'M';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_N))
	{
		wc = 'N';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_O))
	{
		wc = 'O';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_P))
	{
		wc = 'P';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_Q))
	{
		wc = 'Q';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_R))
	{
		wc = 'R';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_S))
	{
		wc = 'S';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_T))
	{
		wc = 'T';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_U))
	{
		wc = 'U';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_X))
	{
		wc = 'X';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_Y))
	{
		wc = 'Y';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_W))
	{
		wc = 'W';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_Z))
	{
		wc = 'Z';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_SPACE))
	{
		wc = ' ';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_1))
	{
		wc = '1';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_2))
	{
		wc = '2';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_3))
	{
		wc = '3';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_4))
	{
		wc = '4';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_5))
	{
		wc = '5';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_6))
	{
		wc = '6';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_7))
	{
		wc = '7';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_8))
	{
		wc = '8';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_9))
	{
		wc = '9';
	}
	else if (pGameInput->KeyFirstDown(KEY_INPUT_0))
	{
		wc = '0';
	}

	if (wc != 0)
	{
		wcBuf[i] = wc;
		i++;
	}
}