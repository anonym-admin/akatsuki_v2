#include "pch.h"
#include "FontManager.h"
#include "Renderer.h"

/*
==============
FontManager
==============
*/

using namespace D2D1;

AkBool FFontManager::CreateD2D(ID3D12Device* pD3DDevice, ID3D12CommandQueue* pCommandQueue, AkBool bEnableDebugLayer)
{
	// Create D3D11 on D3D12
		// Create an 11 device wrapped around the 12 device and share
		// 12's command queue.
	AkU32	d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

	ID2D1Factory3* pD2DFactory = nullptr;
	ID3D11Device* pD3D11Device = nullptr;
	ID3D11DeviceContext* pD3D11DeviceContext = nullptr;
	ID3D11On12Device* pD3D11On12Device = nullptr;
	if (bEnableDebugLayer)
	{
		d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

	if (FAILED(D3D11On12CreateDevice(pD3DDevice,
		d3d11DeviceFlags,
		nullptr,
		0,
		(IUnknown**)&pCommandQueue,
		1,
		0,
		&pD3D11Device,
		&pD3D11DeviceContext,
		nullptr
	)))
	{
		__debugbreak();
	}
	if (FAILED(pD3D11Device->QueryInterface(IID_PPV_ARGS(&pD3D11On12Device))))
		__debugbreak();

	// Create D2D/DWrite components.
	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void**)&pD2DFactory)))
	{
		__debugbreak();
	}

	IDXGIDevice* pDXGIDevice = nullptr;
	if (FAILED(pD3D11On12Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice))))
	{
		__debugbreak();
	}
	if (FAILED(pD2DFactory->CreateDevice(pDXGIDevice, &_pD2DDevice)))
	{
		__debugbreak();
	}


	if (FAILED(_pD2DDevice->CreateDeviceContext(deviceOptions, &_pD2DDeviceContext)))
	{
		__debugbreak();
	}

	if (pD3D11Device)
	{
		pD3D11Device->Release();
		pD3D11Device = nullptr;
	}
	if (pDXGIDevice)
	{
		pDXGIDevice->Release();
		pDXGIDevice = nullptr;
	}
	if (pD2DFactory)
	{
		pD2DFactory->Release();
		pD2DFactory = nullptr;
	}
	if (pD3D11On12Device)
	{
		pD3D11On12Device->Release();
		pD3D11On12Device = nullptr;
	}
	if (pD3D11DeviceContext)
	{
		pD3D11DeviceContext->Release();
		pD3D11DeviceContext = nullptr;
	}

	return AK_TRUE;
}

AkBool FFontManager::CreateDWrite(ID3D12Device* pD3DDevice, AkU32 TexWidth, AkU32 TexHeight, AkF32 fDPI)
{
	AkBool		bResult = AK_FALSE;

	_uD2DBitmapWidth = TexWidth;
	_uD2DBitmapHeight = TexHeight;

	//InitCustomFont(pCustomFontList, dwCustomFontNum);

	D2D1_SIZE_U	size;
	size.width = TexWidth;
	size.height = TexHeight;

	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			fDPI,
			fDPI
		);

	if (FAILED(_pD2DDeviceContext->CreateBitmap(size, nullptr, 0, &bitmapProperties, &_pD2DTargetBitmap)))
		__debugbreak();

	bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_CPU_READ;
	if (FAILED(_pD2DDeviceContext->CreateBitmap(size, nullptr, 0, &bitmapProperties, &_pD2DTargetBitmapRead)))
		__debugbreak();

	if (FAILED(_pD2DDeviceContext->CreateSolidColorBrush(ColorF(ColorF::White), &_pWhiteBrush)))
		__debugbreak();
	if (FAILED(_pD2DDeviceContext->CreateSolidColorBrush(ColorF(ColorF::LightGreen), &_pGreenBrush)))
		__debugbreak();
	if (FAILED(_pD2DDeviceContext->CreateSolidColorBrush(ColorF(ColorF::DarkGray), &_pDarkGrayBrush)))
		__debugbreak();
	if (FAILED(_pD2DDeviceContext->CreateSolidColorBrush(ColorF(ColorF::Gray), &_pGrayBrush)))
		__debugbreak();
	if (FAILED(_pD2DDeviceContext->CreateSolidColorBrush(ColorF(ColorF::Blue), &_pBlueBrush)))
		__debugbreak();

	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5), (IUnknown**)&_pDWFactory);
	if (FAILED(hr))
		__debugbreak();

	bResult = AK_TRUE;

	return bResult;
}

AkBool FFontManager::CreateBitmapFromText(AkI32* pWidth, AkI32* pHeight, IDWriteTextFormat* pTextFormat, const wchar_t* wcText, AkU32 uTextLength, FONT_COLOR_TYPE eFontColor)
{
	AkBool	bResult = AK_FALSE;

	ID2D1DeviceContext* pD2DDeviceContext = _pD2DDeviceContext;
	IDWriteFactory5* pDWFactory = _pDWFactory;
	D2D1_SIZE_F max_size = pD2DDeviceContext->GetSize();
	max_size.width = (float)_uD2DBitmapWidth;
	max_size.height = (float)_uD2DBitmapHeight;

	IDWriteTextLayout* pTextLayout = nullptr;
	if (pDWFactory && pTextFormat)
	{
		if (FAILED(pDWFactory->CreateTextLayout(wcText, uTextLength, pTextFormat, max_size.width, max_size.height, &pTextLayout)))
			__debugbreak();
	}
	DWRITE_TEXT_METRICS metrics = {};
	if (pTextLayout)
	{
		pTextLayout->GetMetrics(&metrics);

		// 타겟설정
		pD2DDeviceContext->SetTarget(_pD2DTargetBitmap);

		// 안티앨리어싱모드 설정
		pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

		// 텍스트 렌더링
		pD2DDeviceContext->BeginDraw();

		pD2DDeviceContext->Clear(D2D1::ColorF(D2D1::ColorF::Black));
		pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		pD2DDeviceContext->DrawTextLayout(D2D1::Point2F(0.0f, 0.0f), pTextLayout, GetSolidColorBrush(eFontColor));

		// We ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
		// is lost. It will be handled during the next call to Present.
		pD2DDeviceContext->EndDraw();

		// 안티앨리어싱 모드 복구    
		pD2DDeviceContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);
		pD2DDeviceContext->SetTarget(nullptr);

		// 레이아웃 오브젝트 필요없으니 Release
		pTextLayout->Release();
		pTextLayout = nullptr;
	}
	AkI32 width = (AkI32)ceil(metrics.width);
	AkI32 height = (AkI32)ceil(metrics.height);

	D2D1_POINT_2U	destPos = {};
	D2D1_RECT_U		srcRect = { 0, 0, (UINT32)width, (UINT32)height };
	if (FAILED(_pD2DTargetBitmapRead->CopyFromBitmap(&destPos, _pD2DTargetBitmap, &srcRect)))
		__debugbreak();

	*pWidth = width;
	*pHeight = height;

	bResult = AK_TRUE;

	return bResult;
}

ID2D1SolidColorBrush* FFontManager::GetSolidColorBrush(FONT_COLOR_TYPE eFontColor)
{
	ID2D1SolidColorBrush* pBrush = nullptr;

	switch (eFontColor)
	{
	case FONT_COLOR_TYPE::FONT_COLOR_TYPE_WHITE:
		pBrush = _pWhiteBrush;
		break;
	case FONT_COLOR_TYPE::FONT_COLOR_TYPE_GREEN:
		pBrush = _pGreenBrush;
		break;
	case FONT_COLOR_TYPE::FONT_COLOR_TYPE_DARK_GRAY:
		pBrush = _pDarkGrayBrush;
		break;
	case FONT_COLOR_TYPE::FONT_COLOR_TYPE_GRAY:
		pBrush = _pGrayBrush;
		break;
	case FONT_COLOR_TYPE::FONT_COLOR_TYPE_BLUE:
		pBrush = _pBlueBrush;
		break;
	default:
		__debugbreak();
		break;
	}

	return pBrush;
}

void FFontManager::CleanupDWrite()
{
	if (_pD2DTargetBitmap)
	{
		_pD2DTargetBitmap->Release();
		_pD2DTargetBitmap = nullptr;
	}
	if (_pD2DTargetBitmapRead)
	{
		_pD2DTargetBitmapRead->Release();
		_pD2DTargetBitmapRead = nullptr;
	}
	if (_pBlueBrush)
	{
		_pBlueBrush->Release();
		_pBlueBrush = nullptr;
	}
	if (_pGrayBrush)
	{
		_pGrayBrush->Release();
		_pGrayBrush = nullptr;
	}
	if (_pDarkGrayBrush)
	{
		_pDarkGrayBrush->Release();
		_pDarkGrayBrush = nullptr;
	}
	if (_pGreenBrush)
	{
		_pGreenBrush->Release();
		_pGreenBrush = nullptr;
	}
	if (_pWhiteBrush)
	{
		_pWhiteBrush->Release();
		_pWhiteBrush = nullptr;
	}
	if (_pDWFactory)
	{
		_pDWFactory->Release();
		_pDWFactory = nullptr;
	}
}

void FFontManager::CleanupD2D()
{
	if (_pD2DDeviceContext)
	{
		_pD2DDeviceContext->Release();
		_pD2DDeviceContext = nullptr;
	}
	if (_pD2DDevice)
	{
		_pD2DDevice->Release();
		_pD2DDevice = nullptr;
	}
}

void FFontManager::Cleanup()
{
	CleanupDWrite();
	CleanupD2D();
}

AkBool FFontManager::Initialize(FRenderer* pRenderer, ID3D12CommandQueue* pCommandQueue, AkU32 uWidth, AkU32 uHeight, AkBool bEnableDebugLayer)
{
	ID3D12Device* pD3DDevice = pRenderer->GetDevice();
	CreateD2D(pD3DDevice, pCommandQueue, bEnableDebugLayer);

	AkF32 fDPI = pRenderer->GetDpi();
	CreateDWrite(pD3DDevice, uWidth, uHeight, fDPI);

	return AK_TRUE;
}

FontHandle_t* FFontManager::CreateFontObject(const wchar_t* wcFontFamilyName, AkF32 fFontSize)
{
	IDWriteTextFormat* pTextFormat = nullptr;
	IDWriteFactory5* pDWFactory = _pDWFactory;
	IDWriteFontCollection1* pFontCollection = nullptr;

	// The logical size of the font in DIP("device-independent pixel") units.A DIP equals 1 / 96 inch.

	if (pDWFactory)
	{
		if (FAILED(pDWFactory->CreateTextFormat(
			wcFontFamilyName,
			pFontCollection,                       // Font collection (nullptr sets it to use the system font collection).
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			fFontSize,
			DEFULAT_LOCALE_NAME,
			&pTextFormat
		)))
		{
			__debugbreak();
		}
	}
	FontHandle_t* pFontHandle = new FontHandle_t;
	memset(pFontHandle, 0, sizeof(FontHandle_t));
	wcscpy_s(pFontHandle->wchFontFamilyName, wcFontFamilyName);
	pFontHandle->fFontSize = fFontSize;

	if (pTextFormat)
	{
		if (FAILED(pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)))
			__debugbreak();


		if (FAILED(pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)))
			__debugbreak();
	}

	pFontHandle->pTextFormat = pTextFormat;

	return pFontHandle;
}

void FFontManager::DeleteFontObject(FontHandle_t* pFontHandle)
{
	if (pFontHandle->pTextFormat)
	{
		pFontHandle->pTextFormat->Release();
		pFontHandle->pTextFormat = nullptr;
	}
	delete pFontHandle;
}

AkBool FFontManager::WriteTextToBitmap(AkU8* pDestImage, AkU32 uDestWidth, AkU32 uDestHeight, AkU32 uDestPitch, AkI32* pWidth, AkI32* pHeight, FontHandle_t* pFontHandle, const wchar_t* wcText, AkU32 uTextLength,  FONT_COLOR_TYPE eFontColor)
{
	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;

	AkBool bResult = CreateBitmapFromText(&iTextWidth, &iTextHeight, pFontHandle->pTextFormat, wcText, uTextLength, eFontColor);
	if (bResult)
	{
		if (iTextWidth > (AkI32)uDestWidth)
			iTextWidth = (AkI32)uDestWidth;

		if (iTextHeight > (AkI32)uDestHeight)
			iTextHeight = (AkI32)uDestHeight;

		D2D1_MAPPED_RECT	mappedRect;
		if (FAILED(_pD2DTargetBitmapRead->Map(D2D1_MAP_OPTIONS_READ, &mappedRect)))
			__debugbreak();

		AkU8* pDest = pDestImage;
		char* pSrc = (char*)mappedRect.bits;

		for (AkU32 y = 0; y < (AkU32)iTextHeight; y++)
		{
			memcpy(pDest, pSrc, iTextWidth * 4);
			pDest += uDestPitch;
			pSrc += mappedRect.pitch;
		}
		_pD2DTargetBitmapRead->Unmap();
	}
	*pWidth = iTextWidth;
	*pHeight = iTextHeight;

	return bResult;
}

FFontManager::FFontManager()
{
}

FFontManager::~FFontManager()
{
	Cleanup();
}
