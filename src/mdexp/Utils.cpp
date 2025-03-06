#include "Utils.h"
#include <stb_image.h>
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION

#include <d3d11.h>
#include <wrl/client.h>
#include <wincodec.h>
#include <DirectXTex.h>
#pragma comment(lib, "d3d11.lib")

void GetFileName(char dest[], const char* src)
{
	size_t len = strlen(src);
	char* temp = new char[len + 1];
	memset(temp, 0, len + 1);

	for (size_t i = len - 1; i >= 0; i--)
	{
		if (src[i] == '/')
		{
			break;
		}
		temp[i] = src[i];
	}

	size_t j = 0;
	for (size_t i = 0; i < len; i++)
	{
		if (temp[i] != 0)
		{
			dest[j++] = temp[i];
		}
	}

	delete[] temp;
}

void ConvertFileExtensionToDDS(wchar_t* wcFilename)
{
	if (!wcscmp(wcFilename, L"Empty"))
	{
		return;
	}
	if (!wcscmp(wcFilename, L""))
	{
		return;
	}

	AkU32 uLen = (AkU32)wcslen(wcFilename);
	wcFilename[uLen - 1] = 's';
	wcFilename[uLen - 2] = 'd';
	wcFilename[uLen - 3] = 'd';
}

void NomalizeModelData(MeshData_t* pMeshData, AkU32 uMeshDataNum, const AkF32 fScaleLength, AkBool bIsAnim , Matrix* pDefaultMatrix)
{
	Vector3 vMax = Vector3(-1000.0f, -1000.0f, -1000.0f);
	Vector3 vMin = Vector3(1000.0f, 1000.0f, 1000.0f);

	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		MeshData_t tMeshData = pMeshData[i];
		for (AkU32 j = 0; j < tMeshData.uVerticeNum; j++)
		{
			if (!bIsAnim)
			{
				Vertex_t v = tMeshData.pVertices[j];
				vMax.x = DirectX::XMMax(v.vPosition.x, vMax.x);
				vMin.x = DirectX::XMMin(v.vPosition.x, vMin.x);
				vMax.y = DirectX::XMMax(v.vPosition.y, vMax.y);
				vMin.y = DirectX::XMMin(v.vPosition.y, vMin.y);
				vMax.z = DirectX::XMMax(v.vPosition.z, vMax.z);
				vMin.z = DirectX::XMMin(v.vPosition.z, vMin.z);
			}
			else
			{
				SkinnedVertex_t v = tMeshData.pSkinnedVertices[j];
				vMax.x = DirectX::XMMax(v.vPosition.x, vMax.x);
				vMin.x = DirectX::XMMin(v.vPosition.x, vMin.x);
				vMax.y = DirectX::XMMax(v.vPosition.y, vMax.y);
				vMin.y = DirectX::XMMin(v.vPosition.y, vMin.y);
				vMax.z = DirectX::XMMax(v.vPosition.z, vMax.z);
				vMin.z = DirectX::XMMin(v.vPosition.z, vMin.z);
			}
		}
	}

	AkF32 dx = vMax.x - vMin.x, dy = vMax.y - vMin.y, dz = vMax.z - vMin.z;
	AkF32 scale = fScaleLength / DirectX::XMMax(DirectX::XMMax(dx, dy), dz);
	Vector3 translation = -(vMax + vMin) * 0.5f;

	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		MeshData_t tMeshData = pMeshData[i];
		for (AkU32 j = 0; j < tMeshData.uVerticeNum; j++)
		{
			if (bIsAnim)
			{
				pMeshData[i].pSkinnedVertices[j].vPosition = (pMeshData[i].pSkinnedVertices[j].vPosition + translation) * scale;
			}
			else
			{
				pMeshData[i].pVertices[j].vPosition = (pMeshData[i].pVertices[j].vPosition + translation) * scale;
			}
		}
	}

	if (bIsAnim)
	{
		*pDefaultMatrix = Matrix::CreateTranslation(translation) * Matrix::CreateScale(scale);
	}
}

static HRESULT SaveDDSTextureToFile(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, ID3D11Resource* texture, LPCWSTR destFile)
{
	using Microsoft::WRL::ComPtr;

	ComPtr<ID3D11Texture2D> texture2D;
	HRESULT hr = texture->QueryInterface(IID_PPV_ARGS(&texture2D));
	if (FAILED(hr)) return hr;

	DirectX::ScratchImage scratchImage;
	hr = DirectX::CaptureTexture(d3dDevice, d3dContext, texture2D.Get(), scratchImage);
	if (FAILED(hr)) return hr;

	hr = DirectX::SaveToDDSFile(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), DirectX::DDS_FLAGS_NONE, destFile);
	return hr;
}

HRESULT PNG_To_DDS(wstring filename)
{
	using Microsoft::WRL::ComPtr;

	// Direct3D 11 장치 초기화
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL featureLevel;
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dContext;
	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 1, D3D11_SDK_VERSION, &d3dDevice, &featureLevel, &d3dContext)))
		return E_FAIL;

	// WIC(Windows Imaging Component) 팩토리 초기화
	ComPtr<IWICImagingFactory> wicFactory;
	if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
		return E_FAIL;
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory))))
		return E_FAIL;

	// PNG 파일 디코딩
	ComPtr<IWICBitmapDecoder> decoder;
	if (FAILED(wicFactory->CreateDecoderFromFilename(wstring(filename + L".png").c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
		return E_FAIL;
	ComPtr<IWICBitmapFrameDecode> frame;
	if (FAILED(decoder->GetFrame(0, &frame)))
		return E_FAIL;

	// 비트맵 원본 형식으로 변환
	ComPtr<IWICFormatConverter> converter;
	if (FAILED(wicFactory->CreateFormatConverter(&converter)))
		return E_FAIL;
	if (FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)))
		return E_FAIL;

	UINT width, height;
	if (FAILED(frame->GetSize(&width, &height)))
		return E_FAIL;

	// 텍스처 생성
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = nullptr;
	initData.SysMemPitch = width * 4; // 4 bytes per pixel (RGBA)
	initData.SysMemSlicePitch = 0;

	// 데이터를 PNG로부터 읽어옴
	std::vector<BYTE> imageData(width * height * 4);
	if (FAILED(converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(imageData.size()), imageData.data())))
		return E_FAIL;
	initData.pSysMem = imageData.data();

	ComPtr<ID3D11Texture2D> texture;
	if (FAILED(d3dDevice->CreateTexture2D(&desc, &initData, &texture)))
		return E_FAIL;

	// DDS 파일로 저장
	if (FAILED(SaveDDSTextureToFile(d3dDevice.Get(), d3dContext.Get(), texture.Get(), wstring(filename + L".dds").c_str()))) // SaveDDSTextureToFile 함수는 별도로 구현 필요
		return E_FAIL;

	return S_OK;
}