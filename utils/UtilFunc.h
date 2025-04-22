#pragma once

#include <string>
#include <DirectXTex.h>
#include "common/CommonMeshData.h"

AkBool ReadBitmapFile(const wchar_t* wcFilename, AkU8** pDestImage, AkU32* pWidth, AkU32* pHeight);
void ReadImage(const wchar_t* pFilename, AkU8** ppOutImage, AkU32* pOutWidth, AkU32* pOutHeight, DXGI_FORMAT* pOutFormat = nullptr);
void SaveDDS(const wchar_t* pFilename, AkBool bGenerateMipMap);
void SaveDDS(const wchar_t* pFilename, AkU8* pImage, AkU32 uWidth, AkU32 uHeight, DXGI_FORMAT Format);
void ImageToPixel(AkU8* pImage, Vector4* pPixels, AkU32 UPixelSize);
void CalcColliderMinMax(MeshData_t* pMeshData, AkU32 uMeshDataNum, Vector3* pOutMin, Vector3* pOutMax);
void QuternionToEuler(Quaternion q, AkF32& yaw, AkF32& pitch, AkF32& roll);
void QuternionToEuler(Quaternion q, Vector3& ypr);