#include "pch.h"
#include "BtnUI.h"
#include "TextUI.h"

/*
===============
Button UI
===============
*/

UBtnUI::UBtnUI()
{
}

UBtnUI::~UBtnUI()
{
	CleanUp();
}

AkBool UBtnUI::Initialize(UApplication* pApp, const wchar_t* wcTexFileName, AkU32 uPosX, AkU32 uPosY, AkU32 uTexWidth, AkU32 uTexHeight)
{
	if (!UPanelUI::Initialize(pApp, wcTexFileName, uPosX, uPosY, uTexWidth, uTexHeight))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pApp = pApp;

	return AK_TRUE;
}

void UBtnUI::Render()
{
	UPanelUI::Render();


}

void UBtnUI::MouseOn()
{
}

void UBtnUI::MouseLBtnDown()
{
	_bIsLBtnDown = AK_TRUE;

	printf("Btn Mouse Btn Down\n");
}

void UBtnUI::MouseLBtnUp()
{
	_bIsLBtnDown = AK_FALSE;

	printf("Btn Mouse Btn Up\n");
}

void UBtnUI::MouseLBtnClick()
{
	printf("Btn Mouse Btn Click\n");

	(_pApp->*_pExitGameFunc)();
}

void UBtnUI::CleanUp()
{
}
