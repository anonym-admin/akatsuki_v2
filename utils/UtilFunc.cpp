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

std::string GetFilePath(const std::string& fullPath)
{
	size_t pos = fullPath.find_last_of("/\\");
	if (pos == std::string::npos) {
		return "";
	}
	return fullPath.substr(0, pos + 1);
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