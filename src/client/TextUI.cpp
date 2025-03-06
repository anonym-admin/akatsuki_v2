#include "pch.h"
#include "TextUI.h"
#include "Application.h"

/*
===========
Text UI
===========
*/

UTextUI::UTextUI()
{
}

UTextUI::~UTextUI()
{
	CleanUp();
}

AkBool UTextUI::Initialize(UApplication* pApp, AkU32 uTextTextureWidth, AkU32 uTextTextureHeight, const wchar_t* wcFontFamilyName, AkF32 fFontSize)
{
	if (!UUI::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	IRenderer* pRenderer = pApp->GetRenderer();

	// Create text texture image.
	_uTextTextureWidth = uTextTextureWidth;
	_uTextTextureHeight = uTextTextureHeight;
	_pTextTextureImage = (AkU8*)malloc(_uTextTextureWidth * _uTextTextureHeight * 4);
	_pTextTextureHandle = pRenderer->CreateDynamicTexture(_uTextTextureWidth, _uTextTextureHeight);
	memset(_pTextTextureImage, 0, _uTextTextureWidth * _uTextTextureHeight * 4);

	// Create font.
	_pFontObj = pRenderer->CreateFontObject(wcFontFamilyName, fFontSize);

	SetResolution(uTextTextureWidth, uTextTextureHeight);

	return AK_TRUE;
}

void UTextUI::WriteText(const wchar_t* wcText)
{
	IRenderer* pRenderer = GetApp()->GetRenderer();

	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;
	AkU32 uTextLen = (AkU32)wcslen(wcText);

	if (wcscmp(_wcText, wcText))
	{
		// 텍스트가 변경된 경우
		memset(_pTextTextureImage, 0, _uTextTextureWidth * _uTextTextureHeight * 4);
		pRenderer->WriteTextToBitmap(_pTextTextureImage, _uTextTextureWidth, _uTextTextureHeight, _uTextTextureWidth * 4, &iTextWidth, &iTextHeight, _pFontObj, wcText, uTextLen);
		pRenderer->UpdateTextureWidthImage(_pTextTextureHandle, _pTextTextureImage, _uTextTextureWidth, _uTextTextureHeight);
		wcscpy_s(_wcText, wcText);
	}
}

void UTextUI::Render()
{
	IRenderer* pRenderer = GetApp()->GetRenderer();

	pRenderer->RenderSpriteWithTex(sm_pCommonSpriteObj, _iPosX, _iPosY, _fScaleX, _fScaleY, _bUseRect ? &_tRect : nullptr, _fDepth, _pTextTextureHandle, &_vFontColor);

	RenderChildUI();
}

void UTextUI::MouseOn()
{
}

void UTextUI::MouseLBtnDown()
{
}

void UTextUI::MouseLBtnUp()
{
}

void UTextUI::MouseLBtnClick()
{
}

void UTextUI::CleanUp()
{
	IRenderer* pRenderer = GetApp()->GetRenderer();

	if (_pFontObj)
	{
		pRenderer->DestroyFontObject(_pFontObj);
		_pFontObj = nullptr;
	}
	if (_pTextTextureHandle)
	{
		pRenderer->DestroyTexture(_pTextTextureHandle);
		_pTextTextureHandle = nullptr;
	}
	if (_pTextTextureImage)
	{
		free(_pTextTextureImage);
		_pTextTextureImage = nullptr;
	}
}
