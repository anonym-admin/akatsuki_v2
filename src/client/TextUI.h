#pragma once

#include "UI.h"

/*
===========
Text UI
===========
*/

class UTextUI : public UUI 
{
public:
	UTextUI();
	~UTextUI();

	AkBool Initialize(UApplication* pApp, AkU32 uTextTextureWidth, AkU32 uTextTextureHeight, const wchar_t* wcFontFamilyName, AkF32 fFontSize);
	void WriteText(const wchar_t* wcText);
	void SetFontColor(const Vector3* pFontColor) { _vFontColor = *pFontColor; }

	virtual void Render() override;

	virtual void MouseOn() override;
	virtual void MouseLBtnDown() override;
	virtual void MouseLBtnUp() override;
	virtual void MouseLBtnClick() override;

private:
	virtual void CleanUp() override;

private:
	AkU32 _uTextTextureWidth = 0;
	AkU32 _uTextTextureHeight = 0;
	AkU8* _pTextTextureImage = nullptr;
	void* _pTextTextureHandle = nullptr;
	wchar_t _wcText[256] = {};
	void* _pFontObj = nullptr;
	Vector3 _vFontColor = Vector3(1.0f);
};
