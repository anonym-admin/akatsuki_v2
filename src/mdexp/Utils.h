#pragma once

#include "common/AkMeshData.h"
#include <string>

using std::wstring;

void GetFileName(char dest[], const char* src);
void ConvertFileExtensionToDDS(wchar_t* wcFilename);
void NomalizeModelData(MeshData_t* pMeshData, AkU32 uMeshDataNum, const AkF32 fScaleLength, AkBool bIsAnim = false, Matrix* pDefaultMatrix = nullptr);
HRESULT PNG_To_DDS(wstring filename);