#include "pch.h"
#include "UtilFunc.h"
#include <stdio.h>
#include <Windows.h>

int Clamp(const int iValue, const int iMin, const int iMax)
{
	if (iValue < iMin)
	{
		return iMin;
	}
	else if (iValue >= iMax)
	{
		return iMax;
	}
	else
	{
		return iValue;
	}
}

float Clamp(const float fValue, const float fMin, const float fMax)
{
	if (fValue < fMin)
	{
		return fMin;
	}
	else if (fValue >= fMax)
	{
		return fMax;
	}
	else
	{
		return fValue;
	}
}

double Clamp(const double dValue, const double dMin, const double dMax)
{
	if (dValue < dMin)
	{
		return dMin;
	}
	else if (dValue >= dMax)
	{
		return dMax;
	}
	else
	{
		return dValue;
	}
}

bool ReadBitmapFile(const wchar_t* wcFilename, unsigned __int8** pDestImage, unsigned __int32 *pWidth, unsigned __int32* pHeight)
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

void ReadImage(const wchar_t* pFilename, unsigned __int8** ppOutImage, unsigned __int32* pOutWidth, unsigned __int32* pOutHeight)
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
}

void ImageToPixel(unsigned __int8* pImage, Vector4* pPixels, unsigned int iPixelSize)
{
	const float fScale = 1.0f / 255.0f;

	for (unsigned int i = 0; i < iPixelSize / 4; i++)
	{
		pPixels[i].x = pImage[4 * i + 0] * fScale;
		pPixels[i].y = pImage[4 * i + 1] * fScale;
		pPixels[i].z = pImage[4 * i + 2] * fScale;
		pPixels[i].w = pImage[4 * i + 3] * fScale;
	}
}

float GenterateRandomFloat(float fMin, float fMax)
{
	float fScale = rand() / (float)RAND_MAX;
	return fMin + fScale * (fMax - fMin);
}

std::string GetFileExtension(const std::string& filePath)
{
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos == std::string::npos) {
		return "";
	}
	size_t slashPos = filePath.find_last_of("/\\");
	if (slashPos != std::string::npos && slashPos > dotPos) {
		return "";
	}
	return filePath.substr(dotPos + 1);
}

std::string GetFilePath(const std::string& filePath)
{
	size_t pos = filePath.find_last_of("/\\");
	if (pos == std::string::npos) {
		return "";
	}
	return filePath.substr(0, pos + 1);
}

std::string GetFileName(const std::string& filePath)
{
	size_t pos = filePath.find_last_of("/\\");
	if (pos == std::string::npos) {
		return "";
	}
	return filePath.substr(pos + 1);
}

std::wstring GetFileExtension(const std::wstring& filePath)
{
	size_t dotPos = filePath.find_last_of(L'.');
	if (dotPos == std::wstring::npos) {
		return L"";
	}
	size_t slashPos = filePath.find_last_of(L"/\\");
	if (slashPos != std::string::npos && slashPos > dotPos) {
		return L"";
	}
	return filePath.substr(dotPos + 1);
}

std::wstring GetFilePath(const std::wstring& fullPath)
{
	size_t pos = fullPath.find_last_of(L"/\\");
	if (pos == std::string::npos) {
		return L"";
	}
	return fullPath.substr(0, pos + 1);
}

std::wstring GetFileName(const std::wstring& filePath)
{
	size_t pos = filePath.find_last_of(L"/\\");
	if (pos == std::string::npos) {
		return L"";
	}
	return filePath.substr(pos + 1);
}

std::string GetFileNmaeExcludeExt(const std::string& fileName)
{
	size_t pos = fileName.find_last_of(".");
	if (pos == std::string::npos) {
		return "";
	}
	return fileName.substr(0, pos);
}

std::wstring GetFileNmaeExcludeExt(const std::wstring& fileName)
{
	size_t pos = fileName.find_last_of(L".");
	if (pos == std::string::npos) {
		return L"";
	}
	return fileName.substr(0, pos + 1);
}

std::wstring ToWString(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}

std::string ToString(const std::wstring& str)
{
	return std::string(str.begin(), str.end());
}

void CalcColliderMinMax(MeshData_t* pMeshData, AkU32 uMeshDataNum, Vector3* pOutMin, Vector3* pOutMax)
{
	Vector3 vMin = Vector3(AK_MAX_F32);
	Vector3 vMax = Vector3(-AK_MAX_F32);
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < pMeshData[i].uVerticeNum; j++)
		{
			vMin.x = min(vMin.x, pMeshData[i].pVertices[j].vPosition.x);
			vMin.y = min(vMin.y, pMeshData[i].pVertices[j].vPosition.y);
			vMin.z = min(vMin.z, pMeshData[i].pVertices[j].vPosition.z);

			vMax.x = max(vMax.x, pMeshData[i].pVertices[j].vPosition.x);
			vMax.y = max(vMax.y, pMeshData[i].pVertices[j].vPosition.y);
			vMax.z = max(vMax.z, pMeshData[i].pVertices[j].vPosition.z);
		}
	}

	*pOutMin = vMin;
	*pOutMax = vMax;
}