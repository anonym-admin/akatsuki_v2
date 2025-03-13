#include "pch.h"
#include "UIImage.h"

/*
==========
UI Image
==========
*/

Vector3 UIImage::IMAGE_COLOR_NORMAL = Vector3(1.0f);
Vector3 UIImage::IMAGE_COLOR_DOWN_01 = Vector3(0.5f);
Vector3 UIImage::IMAGE_COLOR_DOWN_02 = Vector3(0.25f);

UIImage::UIImage(const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY)
{
	if (!Initialize(wcTextureFilename, iTexPosX, iTexPosY, iTexOffsetX, iTexOffsetY, iRenderPosX, iRenderPosY))
	{
		__debugbreak();
	}
}

UIImage::~UIImage()
{
	CleanUp();
}

AkBool UIImage::Initialize(const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY)
{
	_iRenderPosX = iRenderPosX;
	_iRenderPosY = iRenderPosY;

	_iWidth = (iTexOffsetX - iTexPosX);
	_iHeight = (iTexOffsetY - iTexPosY);

	_pSprite = GRenderer->CreateSpriteObjectWidthTex(wcTextureFilename, iTexPosX, iTexPosY, iTexOffsetX, iTexOffsetY); // Texture 상의 위치와 크기
	_pSprite->SetDrawBackground(AK_TRUE);

	return AK_TRUE;
}

void UIImage::Render()
{
	GRenderer->RenderSprite(_pSprite, _iRenderPosX, _iRenderPosY, 1.0f, 1.0f, 0.0f, &_vColor);
}

void UIImage::Rect(AkI32* pOutLeft, AkI32* pOutTop, AkI32* pOutRight, AkI32* pOutBottom)
{
	*pOutLeft = _iRenderPosX;
	*pOutTop = _iRenderPosY;
	*pOutRight = _iRenderPosX + _iWidth;
	*pOutBottom = _iRenderPosY + _iHeight;
}

void UIImage::CleanUp()
{
	if (_pSprite)
	{
		_pSprite->Release();
		_pSprite = nullptr;
	}
}
