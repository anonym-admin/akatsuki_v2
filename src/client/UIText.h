#pragma once

/*
==========
Text UI
==========
*/

class UIText
{
public:
	UIText(AkU32 uTextTextureWidth, AkU32 uTextTextureHeight, const wchar_t* wcFontFamilyName, AkF32 fFontSize);
	~UIText();

	AkBool Initialize(AkU32 uTextTextureWidth, AkU32 uTextTextureHeight, const wchar_t* wcFontFamilyName, AkF32 fFontSize);
	void WriteText(const wchar_t* wcText);

	void Render();

private:
	void CleanUp();

private:
	AkU32 _uTextTextureWidth = 0;
	AkU32 _uTextTextureHeight = 0;
	AkU8* _pTextTextureImage = nullptr;
	void* _pTextTextureHandle = nullptr;
	wchar_t _wcText[256] = {};
	void* _pFontObj = nullptr;
};

