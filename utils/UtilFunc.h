#pragma once

#include <string>
#include <DirectXTex.h>
#include "common/CommonMeshData.h"

void CalcColliderMinMax(MeshData_t* pMeshData, AkU32 uMeshDataNum, Vector3* pOutMin, Vector3* pOutMax);
bool ReadBitmapFile(const wchar_t* wcFilename, unsigned __int8** pDestImage, unsigned __int32* pWidth, unsigned __int32* pHeight);
void ReadImage(const wchar_t* pFilename, unsigned __int8** ppOutImage, unsigned __int32* pOutWidth, unsigned __int32* pOutHeight);
void ImageToPixel(unsigned __int8* pImage, Vector4* pPixels, unsigned int iPixelSize);