#pragma once

/*
==================
ModelImporter
==================
*/

class UAnimation;
class UApplication;

class UModelImporter
{
public:
	AkBool Load(UApplication* pApp, const wchar_t* wcBasePath, const wchar_t* wcFilename, AkBool bForAnim = false);
	AkBool LoadAnimation(UApplication* pApp, const wchar_t* wcBasePath, const wchar_t* wcFilename, AkU32 uBoneNum);
	MeshData_t* GetMeshData() { return _pMeshData; }
	AkU32 GetMeshDataNum() { return _uMeshDataNum; }
	const Matrix* GetBoneOffsetTransformList();
	const AkI32* GetBoneHierarchyList();
	AkU32 GetBoneNum();
	struct AnimationClip_t* GetAnimationClip() { return _pAnimClip; }

private:
	void LoadMeshDataInfo(FILE* pFp, AkBool bForAnim);
	void LoadMaterialFileName(FILE* pFp);
	void LoadVerticesAndIndices(FILE* pFp, AkBool bForAnim);
	void LoadBoneOffsets(FILE* pFp);
	void LoadBoneHierarchy(FILE* pFp);
	void LoadAnimationClip(FILE* pFp);

	void UpdateTangents();

private:
	MeshData_t* _pMeshData = nullptr;
	AkU32 _uMeshDataNum = 0;
	wchar_t _wcFileExtension[8] = {};
	const wchar_t* _wcBasePath = nullptr;

	Matrix* _pBoneOffsetMatrixList = nullptr;
	AkU32 _uBoneNum = 0;
	AkI32* _uBoneHierarchyList = nullptr;
	AkBool _bIsAnim = AK_FALSE;

	struct AnimationClip_t* _pAnimClip = nullptr;
};
