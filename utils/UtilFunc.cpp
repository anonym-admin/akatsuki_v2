#include "pch.h"
#include "UtilFunc.h"
#include <stdio.h>
#include <Windows.h>
#include "src/client/StringUtils.h"

AkBool ReadBitmapFile(const wchar_t* wcFilename, AkU8** pDestImage, AkU32* pWidth, AkU32* pHeight)
{
	FILE* pFp = nullptr;
	if (_wfopen_s(&pFp, wcFilename, L"rb"))
	{
		__debugbreak();
		return false;
	}

	BITMAPFILEHEADER tBitmapFileHeader = {};
	if (1 != fread(&tBitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, pFp))
	{
		__debugbreak();
		return false;
	}

	BITMAPINFOHEADER tBitmapInfoHeader = {};
	if (1 != fread(&tBitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, pFp))
	{
		__debugbreak();
		return false;
	}

	unsigned __int32 uWidth = tBitmapInfoHeader.biWidth;
	unsigned __int32 uHeight = tBitmapInfoHeader.biHeight;

	unsigned __int32 uImageSize = uWidth * uHeight * 3;

	unsigned __int8* pSrcImage = new unsigned __int8[uImageSize];
	
	fseek(pFp, tBitmapFileHeader.bfOffBits, SEEK_SET);

	if (uImageSize != fread(pSrcImage, 1, uImageSize, pFp))
	{
		__debugbreak();
		return false;
	}

	if (fclose(pFp))
	{
		__debugbreak();
		return false;
	}

	*pDestImage = new unsigned __int8[uImageSize];
	*pWidth = uWidth;
	*pHeight = uHeight;

	memcpy(*pDestImage, pSrcImage, uImageSize);

	delete[] pSrcImage;
	pSrcImage = nullptr;

	return true;
}

void ReadImage(const wchar_t* pFilename, AkU8** ppOutImage, AkU32* pOutWidth, AkU32* pOutHeight, DXGI_FORMAT* pOutFormat)
{
	HRESULT hResult = S_OK;

	DirectX::ScratchImage image;

	std::wstring ext = GetFileExtension(pFilename);
	if (ext == L"dds")
	{
		hResult = DirectX::LoadFromDDSFile(pFilename, DirectX::DDS_FLAGS_NONE, nullptr, image);
	}
	else if (ext == L"bmp")
	{
		hResult = ReadBitmapFile(pFilename, ppOutImage, pOutWidth, pOutHeight);
	}
	else if (ext == L"ext")
	{
		
	}
	else if (ext == L"tga")
	{
		hResult = DirectX::LoadFromTGAFile(pFilename, nullptr, image);
	}
	else
	{
		hResult = DirectX::LoadFromWICFile(pFilename, DirectX::WIC_FLAGS_FORCE_RGB, nullptr, image);
	}

	if (FAILED(hResult))
	{
		__debugbreak();
	}

	uint8_t* src = image.GetPixels();
	unsigned int size = (unsigned int)image.GetPixelsSize();

	uint8_t* dest = new unsigned __int8[size];
	memcpy(dest, src, sizeof(unsigned __int8) * size);

	*ppOutImage = dest;
	*pOutWidth = (unsigned int)image.GetMetadata().width;
	*pOutHeight = (unsigned int)image.GetMetadata().height;
	if(pOutFormat)
	{
		*pOutFormat = image.GetMetadata().format;
	}
}

void SaveDDS(const wchar_t* pFilename, AkBool bGenerateMipMap)
{
	using namespace DirectX;

	ScratchImage image;
	ScratchImage mipChain;

	HRESULT hr;

	std::wstring ext = GetFileExtension(pFilename);

	if (ext == L"dds")
	{
		hr = LoadFromDDSFile(pFilename, DDS_FLAGS_NONE, nullptr, image);
	}
	else if (ext == L"tga")
	{
		hr = LoadFromTGAFile(pFilename, nullptr, image);
	}
	else
	{
		hr = LoadFromWICFile(pFilename, WIC_FLAGS_FORCE_RGB, nullptr, image);
	}

	// TODO
	if(bGenerateMipMap)
	{
		hr = GenerateMipMaps(*image.GetImages(), TEX_FILTER_DEFAULT, 0, mipChain);
		if (FAILED(hr))
		{
			__debugbreak();
		}
	}

	std::wstring filePath = GetFileNmaeExcludeExt(pFilename);
	filePath += L".dds";

	// DDS 파일로 저장
	hr = SaveToDDSFile(*image.GetImages(), DDS_FLAGS_NONE, filePath.c_str());
	if (FAILED(hr)) 
	{
		__debugbreak();
	}
}

void SaveDDS(const wchar_t* pFilename, AkU8* pImage, AkU32 uWidth, AkU32 uHeight, DXGI_FORMAT Format)
{
	using namespace DirectX;

	HRESULT hr;

	DirectX::Image image;
	image.pixels = pImage;
	image.width = uWidth;
	image.height = uHeight;
	image.rowPitch = uWidth * 4;
	image.slicePitch = image.rowPitch * uHeight;
	image.format = Format;

	hr = SaveToDDSFile(image, DDS_FLAGS_NONE, pFilename);
	if (FAILED(hr))
	{
		__debugbreak();
	}
}

void ImageToPixel(AkU8* pImage, Vector4* pPixels, AkU32 UPixelSize)
{
	const float fScale = 1.0f / 255.0f;

	for (unsigned int i = 0; i < UPixelSize / 4; i++)
	{
		pPixels[i].x = pImage[4 * i + 0] * fScale;
		pPixels[i].y = pImage[4 * i + 1] * fScale;
		pPixels[i].z = pImage[4 * i + 2] * fScale;
		pPixels[i].w = pImage[4 * i + 3] * fScale;
	}
}

void CalcColliderMinMax(MeshData_t* pMeshData, AkU32 uMeshDataNum, Vector3* pOutMin, Vector3* pOutMax)
{
	Vector3 vMin = Vector3(AK_MAX_F32);
	Vector3 vMax = Vector3(-AK_MAX_F32);
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		if (pMeshData[i].pVertices)
		{
			for (AkU32 j = 0; j < pMeshData[i].uVerticeNum; j++)
			{
				vMin.x = std::min(vMin.x, pMeshData[i].pVertices[j].vPosition.x);
				vMin.y = std::min(vMin.y, pMeshData[i].pVertices[j].vPosition.y);
				vMin.z = std::min(vMin.z, pMeshData[i].pVertices[j].vPosition.z);

				vMax.x = std::max(vMax.x, pMeshData[i].pVertices[j].vPosition.x);
				vMax.y = std::max(vMax.y, pMeshData[i].pVertices[j].vPosition.y);
				vMax.z = std::max(vMax.z, pMeshData[i].pVertices[j].vPosition.z);
			}
		}
		else if (pMeshData[i].pSkinnedVertices)
		{
			for (AkU32 j = 0; j < pMeshData[i].uVerticeNum; j++)
			{
				vMin.x = std::min(vMin.x, pMeshData[i].pSkinnedVertices[j].vPosition.x);
				vMin.y = std::min(vMin.y, pMeshData[i].pSkinnedVertices[j].vPosition.y);
				vMin.z = std::min(vMin.z, pMeshData[i].pSkinnedVertices[j].vPosition.z);

				vMax.x = std::max(vMax.x, pMeshData[i].pSkinnedVertices[j].vPosition.x);
				vMax.y = std::max(vMax.y, pMeshData[i].pSkinnedVertices[j].vPosition.y);
				vMax.z = std::max(vMax.z, pMeshData[i].pSkinnedVertices[j].vPosition.z);
			}
		}
	}

	*pOutMin = vMin;
	*pOutMax = vMax;
}

void QuternionToEuler(Quaternion q, AkF32& yaw, AkF32& pitch, AkF32& roll)
{
	Vector4 vAxis = Vector4(0.0f);
	AkF32 fAngle = 0.0f;

	DirectX::XMQuaternionToAxisAngle((DirectX::XMVECTOR*)&vAxis, &fAngle, q);

	AkF32 fYaw = 0.0f;
	AkF32 fPitch = 0.0f;
	AkF32 fRoll = 0.0f;

	// 축과 각도 => 오일러 각도 변환
	auto Func = [&]() {
		vAxis.w = 0.0f;
		vAxis.Normalize();
		AkF32 c = cos(fAngle);
		AkF32 s = sin(fAngle);
		AkF32 t = 1 - c;

		AkF32 r11 = t * vAxis.x * vAxis.x + c;
		AkF32 r12 = t * vAxis.x * vAxis.y - s * vAxis.z;
		AkF32 r13 = t * vAxis.x * vAxis.z + s * vAxis.y;
		AkF32 r21 = t * vAxis.x * vAxis.y + s * vAxis.z;
		AkF32 r22 = t * vAxis.y * vAxis.y + c;
		AkF32 r23 = t * vAxis.y * vAxis.z - s * vAxis.x;
		AkF32 r31 = t * vAxis.x * vAxis.z - s * vAxis.y;
		AkF32 r32 = t * vAxis.y * vAxis.z + s * vAxis.x;
		AkF32 r33 = t * vAxis.z * vAxis.z + c;

		fRoll = atan2(r21, r11);
		fYaw = asin(-r31);
		fPitch = atan2(r32, r33);

		// fYaw = atan2(r21, r11); // z => roll
		// fPitch = asin(-r31); 
		// fRoll = atan2(r32, r33);

		AkF32 x = DirectX::XMConvertToDegrees(fPitch);
		AkF32 y = DirectX::XMConvertToDegrees(fYaw);
		AkF32 z = DirectX::XMConvertToDegrees(fRoll);

		wprintf_s(L"%f %f %f", x, y, z);

		AkF32 fTestYaw = DirectX::XMConvertToRadians(-y);
		AkF32 fTestPitch = DirectX::XMConvertToRadians(-x);
		AkF32 fTestRoll = DirectX::XMConvertToRadians(z);

		//// Debugging
		//Matrix mMat0 = Matrix::CreateRotationX(x) * Matrix::CreateRotationY(y) * Matrix::CreateRotationZ(z);
		//Matrix mMat1 = Matrix::CreateFromYawPitchRoll(fYaw, fPitch, fRoll);
		};

	Func();

	yaw = fYaw;
	pitch = fPitch;
	roll = fRoll;
}

void QuternionToEuler(Quaternion q, Vector3& ypr)
{
	AkF32 yaw, pitch, roll;
	QuternionToEuler(q, yaw, pitch, roll);
	ypr = Vector3(yaw, pitch, roll);
}
