#pragma once

/*
==========
UI Image
==========
*/

class UIImage
{
public:
	static Vector3 IMAGE_COLOR_NORMAL;
	static Vector3 IMAGE_COLOR_DOWN_01;
	static Vector3 IMAGE_COLOR_DOWN_02;

	UIImage(const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY);
	virtual ~UIImage();

	AkBool Initialize(const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY);
	void Render();

	void Rect(AkI32* pOutLeft, AkI32* pOutTop, AkI32* pOutRight, AkI32* pOutBottom);
	void SetColor(const Vector3* pColor) { _vColor = *pColor; }

private:
	void CleanUp();

private:
	ISprite* _pSprite = nullptr;
	AkI32 _iRenderPosX = 0;
	AkI32 _iRenderPosY = 0;
	AkI32 _iWidth = 0;
	AkI32 _iHeight = 0;

	Vector3 _vColor = Vector3(1.0f); // White
};

