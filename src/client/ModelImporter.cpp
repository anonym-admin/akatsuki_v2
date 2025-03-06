#include "pch.h"
#include "ModelImporter.h"
#include "Application.h"
#include "SceneLoading.h"
#include "SceneManager.h"
#include "Animation.h"
#include <locale.h>
#include <DirectXMesh.h>


/*
==================
ModelImporter
==================
*/

//wchar_t* ConvertWideChar(char* str)
//{
//	size_t convertedChar = 0;
//	size_t len = strlen(str) + 1;
//
//	wchar_t* wc = new wchar_t[len];
//
//	setlocale(LC_ALL, "");
//	mbstowcs_s(&convertedChar, wc, len, str, _TRUNCATE);
//
//	return wc;
//}

static void GetFileNameExtension(const wchar_t* wcFilename, wchar_t* wcOutFileExt)
{
	AkU32 uNameLength = (AkU32)wcslen(wcFilename);
	AkU32 i = uNameLength - 1;
	for (; i >= 0; i--)
	{
		if (L'.' == wcFilename[i])
		{
			break;
		}
	}
	AkU32 uExtLength = uNameLength - i;
	AkU32 uMemSize = sizeof(wchar_t) * uExtLength;
	memcpy(wcOutFileExt, wcFilename + i, uMemSize);
}

static void ConvertFileExtensionToDDS(wchar_t* wcFilename)
{
	if (!wcscmp(wcFilename, L"Empty"))
	{
		return;
	}

	AkU32 uLen = (AkU32)wcslen(wcFilename);
	wcFilename[uLen - 1] = 's';
	wcFilename[uLen - 2] = 'd';
	wcFilename[uLen - 3] = 'd';
}

static void GetFileNameFromFullPath(wchar_t* wcFullPath)
{
	if (!wcscmp(wcFullPath, L"Empty"))
	{
		return;
	}

	AkU32 uFilePathLength = (AkU32)wcslen(wcFullPath);

	wchar_t* pCopyPath = reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t) * uFilePathLength));
	AkU32 i = uFilePathLength - 1;
	for (; i >= 0; i--)
	{
		if (wcFullPath[i] == L'/')
		{
			break;
		}
	}

	AkU32 uFileNameLength = uFilePathLength - i;
	memcpy(pCopyPath, wcFullPath + i + 1, uFileNameLength * sizeof(wchar_t));

	memset(wcFullPath, 0, uFilePathLength * sizeof(wchar_t));

	memcpy(wcFullPath, pCopyPath, uFileNameLength * sizeof(wchar_t));

	free(pCopyPath);
	pCopyPath = nullptr;
}

AkBool UModelImporter::Load(UApplication* pApp, const wchar_t* wcBasePath, const wchar_t* wcFilename, AkBool bForAnim)
{
	FILE* pFp = nullptr;

	_wcBasePath = wcBasePath;
	_bIsAnim = bForAnim;

	{
		wchar_t wcFullPath[256] = {};
		wcscat_s(wcFullPath, wcBasePath);
		wcscat_s(wcFullPath, wcFilename);

		_wfopen_s(&pFp, wcFullPath, L"rt");

		if (!pFp)
		{
			__debugbreak();
			return AK_FALSE;
		}

		GetFileNameExtension(wcFullPath, _wcFileExtension);
	}

	if (wcscmp(_wcFileExtension, L".md3d"))
	{
		__debugbreak();
		return AK_FALSE;
	}

	LoadMeshDataInfo(pFp, bForAnim);

	LoadMaterialFileName(pFp);

	LoadVerticesAndIndices(pFp, bForAnim);

	LoadBoneOffsets(pFp);

	LoadBoneHierarchy(pFp);

	if (pFp)
	{
		fclose(pFp);
	}

	return AK_TRUE;
}

AkBool UModelImporter::LoadAnimation(UApplication* pApp, const wchar_t* wcBasePath, const wchar_t* wcFilename, AkU32 uBoneNum)
{
	FILE* pFp = nullptr;

	_wcBasePath = wcBasePath;
	_uBoneNum = uBoneNum;

	{
		wchar_t wcFullPath[256] = {};
		wcscat_s(wcFullPath, wcBasePath);
		wcscat_s(wcFullPath, wcFilename);

		_wfopen_s(&pFp, wcFullPath, L"rt");

		if (!pFp)
		{
			__debugbreak();
			return AK_FALSE;
		}

		GetFileNameExtension(wcFullPath, _wcFileExtension);
	}

	if (wcscmp(_wcFileExtension, L".anim"))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Load Animation Info.
	LoadAnimationClip(pFp);

	if (pFp)
	{
		fclose(pFp);
	}

	return AK_TRUE;
}

const Matrix* UModelImporter::GetBoneOffsetTransformList()
{
	if (!_bIsAnim)
	{
		__debugbreak();
		return nullptr;
	}

	return _pBoneOffsetMatrixList;
}

const AkI32* UModelImporter::GetBoneHierarchyList()
{
	if (!_bIsAnim)
	{
		__debugbreak();
		return nullptr;
	}

	return _uBoneHierarchyList;
}

AkU32 UModelImporter::GetBoneNum()
{
	if (!_bIsAnim)
	{
		__debugbreak();
		return 0;
	}

	return _uBoneNum;
}

void UModelImporter::LoadMeshDataInfo(FILE* pFp, AkBool bForAnim)
{
	AkI32 iNumMeshes = 0;
	AkI32 iNumVertices = 0;
	AkI32 iNumIndices = 0;

	char cBuf[_MAX_PATH] = {};
	fscanf_s(pFp, "%s\n", cBuf, _MAX_PATH);
	fscanf_s(pFp, "%s %d\n", cBuf, _MAX_PATH, &iNumMeshes);

	if (!iNumMeshes)
	{
		__debugbreak();
	}

	_pMeshData = reinterpret_cast<MeshData_t*>(malloc(sizeof(MeshData_t) * iNumMeshes));
	_uMeshDataNum = iNumMeshes;

	for (AkU32 i = 0; i < _uMeshDataNum; i++)
	{
		fscanf_s(pFp, "%s %d %s %d\n", cBuf, _MAX_PATH, &iNumVertices, cBuf, _MAX_PATH, &iNumIndices);

		_pMeshData[i].uVerticeNum = iNumVertices;
		_pMeshData[i].uIndicesNum = iNumIndices;
		_pMeshData[i].pVertices = !bForAnim ? reinterpret_cast<Vertex_t*>(malloc(sizeof(Vertex_t) * iNumVertices)) : nullptr;
		_pMeshData[i].pSkinnedVertices = bForAnim ? reinterpret_cast<SkinnedVertex_t*>(malloc(sizeof(SkinnedVertex_t) * iNumVertices)) : nullptr;
		_pMeshData[i].pIndices = reinterpret_cast<AkU32*>(malloc(sizeof(AkU32) * iNumIndices));
	}
}

void UModelImporter::LoadMaterialFileName(FILE* pFp)
{
	wchar_t wcBuf[_MAX_PATH] = {};
	wchar_t wcAlbedoTextureFilename[_MAX_PATH] = {};
	wchar_t wcEmissiveTextureFilename[_MAX_PATH] = {};
	wchar_t wcHeightTextureFilename[_MAX_PATH] = {};
	wchar_t wcNormalTextureFilename[_MAX_PATH] = {};
	wchar_t wcMetallicTextureFilename[_MAX_PATH] = {};
	wchar_t wcRoughnessTextureFilename[_MAX_PATH] = {};
	wchar_t wcAoTextureFilename[_MAX_PATH] = {};
	wchar_t wcOpacityTextureFilename[_MAX_PATH] = {};

	fwscanf_s(pFp, L"%s\n", wcBuf, _MAX_PATH);
	for (AkU32 i = 0; i < _uMeshDataNum; i++)
	{
		fwscanf_s(pFp, L"%s %s\n", wcBuf, _MAX_PATH, wcAlbedoTextureFilename, _MAX_PATH);
		fwscanf_s(pFp, L"%s %s\n", wcBuf, _MAX_PATH, wcEmissiveTextureFilename, _MAX_PATH);
		fwscanf_s(pFp, L"%s %s\n", wcBuf, _MAX_PATH, wcHeightTextureFilename, _MAX_PATH);
		fwscanf_s(pFp, L"%s %s\n", wcBuf, _MAX_PATH, wcNormalTextureFilename, _MAX_PATH);
		fwscanf_s(pFp, L"%s %s\n", wcBuf, _MAX_PATH, wcMetallicTextureFilename, _MAX_PATH);
		fwscanf_s(pFp, L"%s %s\n", wcBuf, _MAX_PATH, wcRoughnessTextureFilename, _MAX_PATH);
		fwscanf_s(pFp, L"%s %s\n", wcBuf, _MAX_PATH, wcAoTextureFilename, _MAX_PATH);
		fwscanf_s(pFp, L"%s %s\n", wcBuf, _MAX_PATH, wcOpacityTextureFilename, _MAX_PATH);

		GetFileNameFromFullPath(wcAlbedoTextureFilename);
		GetFileNameFromFullPath(wcEmissiveTextureFilename);
		GetFileNameFromFullPath(wcHeightTextureFilename);
		GetFileNameFromFullPath(wcNormalTextureFilename);
		GetFileNameFromFullPath(wcMetallicTextureFilename);
		GetFileNameFromFullPath(wcRoughnessTextureFilename);
		GetFileNameFromFullPath(wcAoTextureFilename);
		GetFileNameFromFullPath(wcOpacityTextureFilename);

		ConvertFileExtensionToDDS(wcAlbedoTextureFilename);
		ConvertFileExtensionToDDS(wcEmissiveTextureFilename);
		ConvertFileExtensionToDDS(wcHeightTextureFilename);
		ConvertFileExtensionToDDS(wcNormalTextureFilename);
		ConvertFileExtensionToDDS(wcMetallicTextureFilename);
		ConvertFileExtensionToDDS(wcRoughnessTextureFilename);
		ConvertFileExtensionToDDS(wcAoTextureFilename);
		ConvertFileExtensionToDDS(wcOpacityTextureFilename);

		//// allocate.
		//AkU32 uBasePathLength = (AkU32)wcslen(_wcBasePath);

		//AkU32 uAlbedoFilenameLength = (AkU32)wcslen(wcAlbedoTextureFilename);
		//_pMeshData[i].wcAlbedoTextureFilename = (wchar_t*)malloc(sizeof(wchar_t) * (uBasePathLength + uAlbedoFilenameLength + 1));
		//memset(_pMeshData[i].wcAlbedoTextureFilename, 0, sizeof(wchar_t) * (uBasePathLength + uAlbedoFilenameLength + 1));

		//AkU32 uEmissiveFilenameLength = (AkU32)wcslen(wcEmissiveTextureFilename);
		//_pMeshData[i].wcEmissiveTextureFilename = (wchar_t*)malloc(sizeof(wchar_t) * (uBasePathLength + uEmissiveFilenameLength + 1));
		//memset(_pMeshData[i].wcEmissiveTextureFilename, 0, sizeof(wchar_t) * (uBasePathLength + uEmissiveFilenameLength + 1));

		//AkU32 uHeightFilenameLength = (AkU32)wcslen(wcHeightTextureFilename);
		//_pMeshData[i].wcHeightTextureFilename = (wchar_t*)malloc(sizeof(wchar_t) * (uBasePathLength + uHeightFilenameLength + 1));
		//memset(_pMeshData[i].wcHeightTextureFilename, 0, sizeof(wchar_t) * (uBasePathLength + uHeightFilenameLength + 1));

		//AkU32 uNormalFilenameLength = (AkU32)wcslen(wcNormalTextureFilename);
		//_pMeshData[i].wcNormalTextureFilename = (wchar_t*)malloc(sizeof(wchar_t) * (uBasePathLength + uNormalFilenameLength + 1));
		//memset(_pMeshData[i].wcNormalTextureFilename, 0, sizeof(wchar_t) * (uBasePathLength + uNormalFilenameLength + 1));

		//AkU32 uMetallicFilenameLength = (AkU32)wcslen(wcMetallicTextureFilename);
		//_pMeshData[i].wcMetallicTextureFilename = (wchar_t*)malloc(sizeof(wchar_t) * (uBasePathLength + uMetallicFilenameLength + 1));
		//memset(_pMeshData[i].wcMetallicTextureFilename, 0, sizeof(wchar_t) * (uBasePathLength + uMetallicFilenameLength + 1));

		//AkU32 uRoughnessFilenameLength = (AkU32)wcslen(wcRoughnessTextureFilename);
		//_pMeshData[i].wcRoughnessTextureFilename = (wchar_t*)malloc(sizeof(wchar_t) * (uBasePathLength + uRoughnessFilenameLength + 1));
		//memset(_pMeshData[i].wcRoughnessTextureFilename, 0, sizeof(wchar_t) * (uBasePathLength + uRoughnessFilenameLength + 1));

		//AkU32 uAoFilenameLength = (AkU32)wcslen(wcAoTextureFilename);
		//_pMeshData[i].wcAoTextureFilename = (wchar_t*)malloc(sizeof(wchar_t) * (uBasePathLength + uAoFilenameLength + 1));
		//memset(_pMeshData[i].wcAoTextureFilename, 0, sizeof(wchar_t) * (uBasePathLength + uAoFilenameLength + 1));

		//AkU32 uOpacityFilenameLength = (AkU32)wcslen(wcOpacityTextureFilename);
		//_pMeshData[i].wcOpacityTextureFilename = (wchar_t*)malloc(sizeof(wchar_t) * (uBasePathLength + uOpacityFilenameLength + 1));
		//memset(_pMeshData[i].wcOpacityTextureFilename, 0, sizeof(wchar_t) * (uBasePathLength + uOpacityFilenameLength + 1));

		// copy and cat.
		wcscpy_s(_pMeshData[i].wcAlbedoTextureFilename, _wcBasePath);
		if (!wcscmp(wcAlbedoTextureFilename, L"Empty"))
			wcscpy_s(_pMeshData[i].wcAlbedoTextureFilename, L"");
		else
			wcscat_s(_pMeshData[i].wcAlbedoTextureFilename, wcAlbedoTextureFilename);

		wcscpy_s(_pMeshData[i].wcEmissiveTextureFilename, _wcBasePath);
		if(!wcscmp(wcEmissiveTextureFilename, L"Empty"))
			wcscpy_s(_pMeshData[i].wcEmissiveTextureFilename, L"");
		else
			wcscat_s(_pMeshData[i].wcEmissiveTextureFilename, wcEmissiveTextureFilename);

		wcscpy_s(_pMeshData[i].wcHeightTextureFilename, _wcBasePath);
		if (!wcscmp(wcHeightTextureFilename, L"Empty"))
			wcscpy_s(_pMeshData[i].wcHeightTextureFilename, L"");
		else
			wcscat_s(_pMeshData[i].wcHeightTextureFilename, wcHeightTextureFilename);

		wcscpy_s(_pMeshData[i].wcNormalTextureFilename, _wcBasePath);
		if (!wcscmp(wcNormalTextureFilename, L"Empty"))
			wcscpy_s(_pMeshData[i].wcNormalTextureFilename, L"");
		else
			wcscat_s(_pMeshData[i].wcNormalTextureFilename, wcNormalTextureFilename);

		wcscpy_s(_pMeshData[i].wcMetallicTextureFilename, _wcBasePath);
		if (!wcscmp(wcMetallicTextureFilename, L"Empty"))
			wcscpy_s(_pMeshData[i].wcMetallicTextureFilename, L"");
		else
			wcscat_s(_pMeshData[i].wcMetallicTextureFilename, wcMetallicTextureFilename);

		wcscpy_s(_pMeshData[i].wcRoughnessTextureFilename, _wcBasePath);
		if (!wcscmp(wcRoughnessTextureFilename, L"Empty"))
			wcscpy_s(_pMeshData[i].wcRoughnessTextureFilename, L"");
		else
			wcscat_s(_pMeshData[i].wcRoughnessTextureFilename, wcRoughnessTextureFilename);

		wcscpy_s(_pMeshData[i].wcAoTextureFilename, _wcBasePath);
		if (!wcscmp(wcAoTextureFilename, L"Empty"))
			wcscpy_s(_pMeshData[i].wcAoTextureFilename, L"");
		else
			wcscat_s(_pMeshData[i].wcAoTextureFilename, wcAoTextureFilename);

		wcscpy_s(_pMeshData[i].wcOpacityTextureFilename, _wcBasePath);
		if(!wcscmp(wcOpacityTextureFilename, L"Empty"))
			wcscpy_s(_pMeshData[i].wcOpacityTextureFilename, L"");
		else
			wcscat_s(_pMeshData[i].wcOpacityTextureFilename, wcOpacityTextureFilename);
	}
}

void UModelImporter::LoadVerticesAndIndices(FILE* pFp, AkBool bForAnim)
{
	char cBuf[_MAX_PATH] = {};

	fscanf_s(pFp, "%s\n", cBuf, _MAX_PATH);
	for (AkU32 i = 0; i < _uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < _pMeshData[i].uVerticeNum; j++)
		{
			if (!bForAnim)
			{
				fscanf_s(pFp, "%s %f %f %f\n", cBuf, _MAX_PATH, &_pMeshData[i].pVertices[j].vPosition.x, &_pMeshData[i].pVertices[j].vPosition.y, &_pMeshData[i].pVertices[j].vPosition.z);
				fscanf_s(pFp, "%s %f %f %f\n", cBuf, _MAX_PATH, &_pMeshData[i].pVertices[j].vNormalModel.x, &_pMeshData[i].pVertices[j].vNormalModel.y, &_pMeshData[i].pVertices[j].vNormalModel.z);
				fscanf_s(pFp, "%s %f %f\n", cBuf, _MAX_PATH, &_pMeshData[i].pVertices[j].vTexCoord.x, &_pMeshData[i].pVertices[j].vTexCoord.y);
				fscanf_s(pFp, "%s %f %f %f\n", cBuf, _MAX_PATH, &_pMeshData[i].pVertices[j].vTangentModel.x, &_pMeshData[i].pVertices[j].vTangentModel.y, &_pMeshData[i].pVertices[j].vTangentModel.z);
			}
			else
			{
				fscanf_s(pFp, "%s %f %f %f\n", cBuf, _MAX_PATH, &_pMeshData[i].pSkinnedVertices[j].vPosition.x, &_pMeshData[i].pSkinnedVertices[j].vPosition.y, &_pMeshData[i].pSkinnedVertices[j].vPosition.z);
				fscanf_s(pFp, "%s %f %f %f\n", cBuf, _MAX_PATH, &_pMeshData[i].pSkinnedVertices[j].vNormalModel.x, &_pMeshData[i].pSkinnedVertices[j].vNormalModel.y, &_pMeshData[i].pSkinnedVertices[j].vNormalModel.z);
				fscanf_s(pFp, "%s %f %f\n", cBuf, _MAX_PATH, &_pMeshData[i].pSkinnedVertices[j].vTexCoord.x, &_pMeshData[i].pSkinnedVertices[j].vTexCoord.y);
				fscanf_s(pFp, "%s %f %f %f\n", cBuf, _MAX_PATH, &_pMeshData[i].pSkinnedVertices[j].vTangentModel.x, &_pMeshData[i].pSkinnedVertices[j].vTangentModel.y, &_pMeshData[i].pSkinnedVertices[j].vTangentModel.z);
				fscanf_s(pFp, "%s", cBuf, _MAX_PATH);
				for (AkU32 k = 0; k < 8; k++)
				{
					fscanf_s(pFp, "%f", &_pMeshData[i].pSkinnedVertices[j].fBlendWeights[k]);
				}
				fscanf_s(pFp, "%s", cBuf, _MAX_PATH);
				for (AkU32 k = 0; k < 8; k++)
				{
					AkI32 iNum = 0;
					fscanf_s(pFp, "%d", &iNum);
					_pMeshData[i].pSkinnedVertices[j].uBoneIndices[k] = (AkU32)iNum;
				}
			}
		}
	}
	fscanf_s(pFp, "%s\n", cBuf, _MAX_PATH);
	for (AkU32 i = 0; i < _uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < _pMeshData[i].uIndicesNum; j += 3)
		{
			fscanf_s(pFp, "%u %u %u\n", &_pMeshData[i].pIndices[j], &_pMeshData[i].pIndices[j + 1], &_pMeshData[i].pIndices[j + 2]);
		}
	}

	UpdateTangents();
}

void UModelImporter::LoadBoneOffsets(FILE* pFp)
{
	char cBuf[_MAX_PATH] = {};
	AkU32 uNumBones;

	fscanf_s(pFp, "%s\n", cBuf, _MAX_PATH);
	fscanf_s(pFp, "%s %u\n", cBuf, _MAX_PATH, &uNumBones);

	if (!uNumBones)
	{
		return;
	}

	_uBoneNum = uNumBones;
	_pBoneOffsetMatrixList = reinterpret_cast<Matrix*>(malloc(sizeof(Matrix) * uNumBones));

	for (AkU32 i = 0; i < _uBoneNum; i++)
	{
		fscanf_s(pFp, "%s\n", cBuf, _MAX_PATH);
		fscanf_s(pFp, "%f %f %f %f\n", &_pBoneOffsetMatrixList[i]._11, &_pBoneOffsetMatrixList[i]._12, &_pBoneOffsetMatrixList[i]._13, &_pBoneOffsetMatrixList[i]._14);
		fscanf_s(pFp, "%f %f %f %f\n", &_pBoneOffsetMatrixList[i]._21, &_pBoneOffsetMatrixList[i]._22, &_pBoneOffsetMatrixList[i]._23, &_pBoneOffsetMatrixList[i]._24);
		fscanf_s(pFp, "%f %f %f %f\n", &_pBoneOffsetMatrixList[i]._31, &_pBoneOffsetMatrixList[i]._32, &_pBoneOffsetMatrixList[i]._33, &_pBoneOffsetMatrixList[i]._34);
		fscanf_s(pFp, "%f %f %f %f\n", &_pBoneOffsetMatrixList[i]._41, &_pBoneOffsetMatrixList[i]._42, &_pBoneOffsetMatrixList[i]._43, &_pBoneOffsetMatrixList[i]._44);
	}
}

void UModelImporter::LoadBoneHierarchy(FILE* pFp)
{
	if (!_uBoneNum)
	{
		return;
	}

	_uBoneHierarchyList = reinterpret_cast<AkI32*>(malloc(sizeof(AkI32) * _uBoneNum));

	char cBuf[_MAX_PATH] = {};

	fscanf_s(pFp, "%s\n", cBuf, _MAX_PATH);
	for (AkU32 i = 0; i < _uBoneNum; i++)
	{
		fscanf_s(pFp, "%s %d\n", cBuf, _MAX_PATH, &_uBoneHierarchyList[i]);
	}
}

void UModelImporter::LoadAnimationClip(FILE* pFp)
{
	wchar_t cBuf[_MAX_PATH] = {};
	wchar_t cTakeNmae[_MAX_PATH] = {};
	AkU32 uNumClip = 0;

	fwscanf_s(pFp, L"%s\n", cBuf, _MAX_PATH);
	fwscanf_s(pFp, L"%s %u\n", cBuf, _MAX_PATH, &uNumClip);

	// 정책상 로드할때 클립은 1개를 가정한다.
	if (uNumClip > 1)
	{
		__debugbreak();
		return;
	}

	for (AkU32 i = 0; i < uNumClip; i++)
	{
		fwscanf_s(pFp, L"%s %s\n", cBuf, _MAX_PATH, cTakeNmae, _MAX_PATH);

		AnimationClip_t* pAnimationClip = new AnimationClip_t[uNumClip];
		pAnimationClip->pBoneAnimationList = new BoneAnimation_t[_uBoneNum];
		pAnimationClip->uNumBoneAnimation = _uBoneNum;

		fwscanf_s(pFp, L"%s %u\n", cBuf, _MAX_PATH, &pAnimationClip->uDuration);
		fwscanf_s(pFp, L"%s %u\n", cBuf, _MAX_PATH, &pAnimationClip->uTickPerSecond);
		fwscanf_s(pFp, L"%s\n", cBuf, _MAX_PATH);

		for (AkU32 j = 0; j < _uBoneNum; j++)
		{
			fwscanf_s(pFp, L"%s %s %u\n", cBuf, _MAX_PATH, cBuf, _MAX_PATH, &pAnimationClip->pBoneAnimationList[j].uNumKeyFrame);
			fwscanf_s(pFp, L"%s\n", cBuf, _MAX_PATH);

			if (pAnimationClip->pBoneAnimationList[j].uNumKeyFrame > 0)
			{
				pAnimationClip->pBoneAnimationList[j].pKeyFrameList = new KeyFrame_t[pAnimationClip->pBoneAnimationList[j].uNumKeyFrame];
			}
			else
			{
				pAnimationClip->pBoneAnimationList[j].pKeyFrameList = nullptr;
			}

			for (AkU32 k = 0; k < pAnimationClip->pBoneAnimationList[j].uNumKeyFrame; k++)
			{
				fwscanf_s(pFp, L"%s %f\n", cBuf, _MAX_PATH, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].fTimePos);
				fwscanf_s(pFp, L"%s %f %f %f\n", cBuf, _MAX_PATH, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].vPos.x, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].vPos.y, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].vPos.z);
				fwscanf_s(pFp, L"%s %f %f %f\n", cBuf, _MAX_PATH, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].vScale.x, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].vScale.y, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].vScale.z);
				fwscanf_s(pFp, L"%s %f %f %f %f\n", cBuf, _MAX_PATH, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].qRot.x, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].qRot.y, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].qRot.z, &pAnimationClip->pBoneAnimationList[j].pKeyFrameList[k].qRot.w);
			}

			fwscanf_s(pFp, L"%s\n", cBuf, _MAX_PATH);
		}

		fwscanf_s(pFp, L"%s\n", cBuf, _MAX_PATH);

		_pAnimClip = pAnimationClip;
	}
}

void UModelImporter::UpdateTangents()
{
	for (AkU32 i = 0; i < _uMeshDataNum; i++)
	{
		DirectX::XMFLOAT3* pPositions = new DirectX::XMFLOAT3[_pMeshData[i].uVerticeNum];
		DirectX::XMFLOAT3* pNormals = new DirectX::XMFLOAT3[_pMeshData[i].uVerticeNum];
		DirectX::XMFLOAT2* pTexCoords = new DirectX::XMFLOAT2[_pMeshData[i].uVerticeNum];
		DirectX::XMFLOAT3* pTangents = new DirectX::XMFLOAT3[_pMeshData[i].uVerticeNum];
		DirectX::XMFLOAT3* pBiTangents = new DirectX::XMFLOAT3[_pMeshData[i].uVerticeNum];

		for (AkU32 j = 0; j < _pMeshData[i].uVerticeNum; j++)
		{
			Vertex_t tVertex = {};
			SkinnedVertex_t tSkinnedVertex = {};
			if (_pMeshData[i].pVertices)
			{
				tVertex = _pMeshData[i].pVertices[j];
				pPositions[j] = tVertex.vPosition;
				pNormals[j] = tVertex.vNormalModel;
				pTexCoords[j] = tVertex.vTexCoord;
			}
			if (_pMeshData[i].pSkinnedVertices)
			{
				tSkinnedVertex = _pMeshData[i].pSkinnedVertices[j];
				pPositions[j] = tSkinnedVertex.vPosition;
				pNormals[j] = tSkinnedVertex.vNormalModel;
				pTexCoords[j] = tSkinnedVertex.vTexCoord;
			}
		}

		DirectX::ComputeTangentFrame(_pMeshData[i].pIndices, _pMeshData[i].uIndicesNum / 3, pPositions, pNormals, pTexCoords, (size_t)_pMeshData[i].uVerticeNum, pTangents, pBiTangents);

		if (_pMeshData->pVertices)
		{
			for (AkU32 j = 0; j < _pMeshData[i].uVerticeNum; j++)
			{
				_pMeshData[i].pVertices[j].vTangentModel = pTangents[j];
			}
		}

		if (_pMeshData->pSkinnedVertices)
		{
			for (AkU32 j = 0; j < _pMeshData[i].uVerticeNum; j++)
			{
				_pMeshData[i].pSkinnedVertices[j].vTangentModel = pTangents[j];
			}
		}

		delete[] pBiTangents;
		delete[] pTangents;
		delete[] pTexCoords;
		delete[] pNormals;
		delete[] pPositions;
	}
}





