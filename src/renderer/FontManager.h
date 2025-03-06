#pragma once

#define DEFULAT_LOCALE_NAME		L"ko-kr"

/*
==============
FontManager
==============
*/

class FRenderer;

class FFontManager
{
public:
	FFontManager();
	~FFontManager();

	AkBool Initialize(FRenderer* pRenderer, ID3D12CommandQueue* pCommandQueue, AkU32 uWidth, AkU32 uHeight, AkBool bEnableDebugLayer);
	FontHandle_t* CreateFontObject(const wchar_t* wchFontFamilyName, AkF32 fFontSize);
	void DeleteFontObject(FontHandle_t* pFontHandle);
	AkBool WriteTextToBitmap(AkU8* pDestImage, AkU32 DestWidth, AkU32 DestHeight, AkU32 DestPitch, AkI32* pWidth, AkI32* pHeight, FontHandle_t* pFontHandle, const wchar_t* wcText, AkU32 dwLen, FONT_COLOR_TYPE eFontColor);

private:
	AkBool CreateD2D(ID3D12Device* pD3DDevice, ID3D12CommandQueue* pCommandQueue, AkBool bEnableDebugLayer);
	AkBool CreateDWrite(ID3D12Device* pD3DDevice, AkU32 TexWidth, AkU32 TexHeight, AkF32 fDPI);
	AkBool CreateBitmapFromText(AkI32* pWidth, AkI32* pHeight, IDWriteTextFormat* pTextFormat, const wchar_t* wcText, AkU32 uTextLength, FONT_COLOR_TYPE eFontColor);
	ID2D1SolidColorBrush* GetSolidColorBrush(FONT_COLOR_TYPE eFontColor);
	void CleanupDWrite();
	void CleanupD2D();
	void Cleanup();

private:
	FRenderer* _pRenderer = nullptr;
	ID2D1Device2* _pD2DDevice = nullptr;
	ID2D1DeviceContext2* _pD2DDeviceContext = nullptr;

	ID2D1Bitmap1* _pD2DTargetBitmap = nullptr;
	ID2D1Bitmap1* _pD2DTargetBitmapRead = nullptr;
	IDWriteFontCollection1* _pFontCollection = nullptr;

	ID2D1SolidColorBrush* _pWhiteBrush = nullptr;

	// TODO
	ID2D1SolidColorBrush* _pGreenBrush = nullptr;
	ID2D1SolidColorBrush* _pDarkGrayBrush = nullptr;
	ID2D1SolidColorBrush* _pGrayBrush = nullptr;
	ID2D1SolidColorBrush* _pBlueBrush = nullptr;

	IDWriteFactory5* _pDWFactory = nullptr;
	DWRITE_LINE_METRICS* _pLineMetrics = nullptr;
	AkU32 _uMaxLineMetricsNum = 0;
	AkU32 _uD2DBitmapWidth = 0;
	AkU32 _uD2DBitmapHeight = 0;
};

