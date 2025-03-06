#include "pch.h"
#include "BtnUI.h"
#include "TextUI.h"

/*
===============
Button UI
===============
*/

UBtnUI::UBtnUI(const wchar_t* wcTexFileName, AkU32 uPosX, AkU32 uPosY, AkU32 uTexWidth, AkU32 uTexHeight)
	: UPanelUI(wcTexFileName, uPosX, uPosY, uTexWidth, uTexHeight)
{
}

UBtnUI::~UBtnUI()
{
	CleanUp();
}

AkBool UBtnUI::Initialize(const wchar_t* wcTexFileName, AkU32 uPosX, AkU32 uPosY, AkU32 uTexWidth, AkU32 uTexHeight)
{
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

	(GApp->*_pExitGameFunc)();
}

void UBtnUI::CleanUp()
{
}
