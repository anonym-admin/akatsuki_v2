#pragma once

#include <string>
#include <DirectXTex.h>
#include "common/AkMeshData.h"

int Clamp(const int iValue, const int iMin, const int iMax);
float Clamp(const float fValue, const float fMin, const float fMax);
double Clamp(const double dValue, const double dMin, const double dMax);

float GenterateRandomFloat(float fMin, float fMax);

void CalcColliderMinMax(MeshData_t* pMeshData, AkU32 uMeshDataNum, Vector3* pOutMin, Vector3* pOutMax);

std::string GetFileExtension(const std::string& filePath);
std::string GetFilePath(const std::string& fullPath);
std::wstring GetFileExtension(const std::wstring& filePath);
std::wstring GetFilePath(const std::wstring& fullPath);

bool ReadBitmapFile(const wchar_t* wcFilename, unsigned __int8** pDestImage, unsigned __int32* pWidth, unsigned __int32* pHeight);
void ReadImage(const wchar_t* pFilename, unsigned __int8** ppOutImage, unsigned __int32* pOutWidth, unsigned __int32* pOutHeight);
void ImageToPixel(unsigned __int8* pImage, Vector4* pPixels, unsigned int iPixelSize);