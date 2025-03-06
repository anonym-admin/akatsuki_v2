#pragma once

#include "TextUI.h"

enum class CHAT_THREAD_EVENT_TYPE
{
	CHAT_THREAD_EVENT_START,
	CHAT_THREAD_EVNET_END,
	CHAT_THREAD_EVENT_COUNT,
};

struct CHAT_THREAD_PARAM
{
	void* pApp = nullptr;
	void* pUI = nullptr;
	HANDLE hThread = nullptr;
	HANDLE hEvent[(AkU32)CHAT_THREAD_EVENT_TYPE::CHAT_THREAD_EVENT_COUNT] = {};
};

AkU32 ProcessChatting(void* pChatParam);
void ProcessChatInput(class UGameInput* pGameInput, wchar_t* wcBuf);

/*
==========
Input UI
==========
*/

class UInputUI : public UTextUI
{
public:
	UInputUI();
	~UInputUI();

	AkBool Initialize(UApplication* pApp, AkU32 uTextTextureWidth, AkU32 uTextTextureHeight, const wchar_t* wcFontFamilyName, AkF32 fFontSize);
	virtual void Update(const AkF32 fDeltaTime) override;

private:
	virtual void CleanUp() override;

private:
	CHAT_THREAD_PARAM _tChatParam = {};
};

