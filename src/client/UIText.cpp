#include "pch.h"
#include "UIText.h"

/*
==========
Text UI
==========
*/

UIText::UIText(AkU32 uTextTextureWidth, AkU32 uTextTextureHeight, const wchar_t* wcFontFamilyName, AkF32 fFontSize)
{
    if (!Initialize(uTextTextureWidth, uTextTextureHeight, wcFontFamilyName, fFontSize))
    {
        __debugbreak();
    }
}

UIText::~UIText()
{
    CleanUp();
}

AkBool UIText::Initialize(AkU32 uTextTextureWidth, AkU32 uTextTextureHeight, const wchar_t* wcFontFamilyName, AkF32 fFontSize)
{
    // Create text texture image.
    _uTextTextureWidth = uTextTextureWidth;
    _uTextTextureHeight = uTextTextureHeight;
    _pTextTextureImage = (AkU8*)malloc(_uTextTextureWidth * _uTextTextureHeight * 4);
    _pTextTextureHandle = GRenderer->CreateDynamicTexture(_uTextTextureWidth, _uTextTextureHeight);
    memset(_pTextTextureImage, 0, _uTextTextureWidth * _uTextTextureHeight * 4);

    // Create font.
    _pFontObj = GRenderer->CreateFontObject(wcFontFamilyName, fFontSize);


    return AkBool();
}

void UIText::WriteText(const wchar_t* wcText)
{
    AkI32 iTextWidth = 0;
    AkI32 iTextHeight = 0;
    AkU32 uTextLen = (AkU32)wcslen(wcText);

    if (wcscmp(_wcText, wcText))
    {
        // 텍스트가 변경된 경우
        memset(_pTextTextureImage, 0, _uTextTextureWidth * _uTextTextureHeight * 4);
        GRenderer->WriteTextToBitmap(_pTextTextureImage, _uTextTextureWidth, _uTextTextureHeight, _uTextTextureWidth * 4, &iTextWidth, &iTextHeight, _pFontObj, wcText, uTextLen);
        GRenderer->UpdateTextureWidthImage(_pTextTextureHandle, _pTextTextureImage, _uTextTextureWidth, _uTextTextureHeight);
        wcscpy_s(_wcText, wcText);
    }
}

void UIText::Render()
{
}

void UIText::CleanUp()
{
}
