#include "pch.h"
#include "PanelUI.h"
#include "Application.h"

/*
===========
Panel UI
===========
*/

UPanelUI::UPanelUI(const wchar_t* wcTexFileName, AkU32 uPosX, AkU32 uPosY, AkU32 uTexWidth, AkU32 uTexHeight)
{
	if (!Initialize(wcTexFileName,  uPosX,  uPosY,  uTexWidth,  uTexHeight))
	{
		__debugbreak();
	}
}

UPanelUI::~UPanelUI()
{
	CleanUp();
}

AkBool UPanelUI::Initialize(const wchar_t* wcTexFileName, AkU32 uPosX, AkU32 uPosY, AkU32 uTexWidth, AkU32 uTexHeight)
{
	if (!UUI::Initialize())
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pTexSpriteObj = GRenderer->CreateSpriteObjectWidthTex(wcTexFileName, (AkI32)uPosX, (AkI32)uPosY, (AkI32)uTexWidth, (AkI32)uTexHeight);

	return AK_TRUE;
}

void UPanelUI::MouseOn()
{
}

void UPanelUI::MouseLBtnDown()
{
	_bIsLBtnDown = AK_TRUE;

	printf("Panel Mouse Btn Down\n");
}

void UPanelUI::MouseLBtnUp()
{
	_bIsLBtnDown = AK_FALSE;

	printf("Panel Mouse Btn Up\n");
}

void UPanelUI::MouseLBtnClick()
{
	printf("Panel Mouse Btn Click\n");
}

void UPanelUI::SetDrawBackGround(AkBool bDrawBackGround)
{
	_pTexSpriteObj->SetDrawBackground(bDrawBackGround);
}

void UPanelUI::Render()
{
	if (GRenderer)
	{
		GRenderer->RenderSprite(_pTexSpriteObj, _iPosX, _iPosY, _fScaleX, _fScaleY, _fDepth);
	}

	RenderChildUI();
}

UPanelUI* UPanelUI::Clone()
{
	UPanelUI* pClone = new UPanelUI(*this); // Shallow copy.
	pClone->_bClone = AK_TRUE;
	return pClone;
}

void UPanelUI::CleanUp()
{
	if (_pTexSpriteObj && !_bClone)
	{
		_pTexSpriteObj->Release();
		_pTexSpriteObj = nullptr;
	}
}