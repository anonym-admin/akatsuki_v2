#pragma once

/*
==========
UI Image
==========
*/

class UIImage
{
public:
	UIImage(const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY);
	virtual ~UIImage();

	AkBool Initialize(const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY);
	void Render();

	void Rect(AkI32* pOutLeft, AkI32* pOutTop, AkI32* pOutRight, AkI32* pOutBottom);

private:
	void CleanUp();

private:
	ISprite* _pSprite = nullptr;
	AkI32 _iRenderPosX = 0;
	AkI32 _iRenderPosY = 0;
	AkI32 _iWidth = 0;
	AkI32 _iHeight = 0;
};

