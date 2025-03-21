#pragma once

/*
==================
ModelImporter
==================
*/

class Animation;
struct AnimationClip_t;

class ModelImporter
{
public:
	AkBool Load( const wchar_t* wcBasePath, const wchar_t* wcFilename, AkBool bForAnim = false);
	AkBool LoadAnimation(const wchar_t* wcBasePath, const wchar_t* wcFilename, AkU32 uBoneNum);
	MeshData_t* GetMeshData() { return _pMeshData; }
	AkU32 GetMeshDataNum() { return _uMeshDataNum; }
	const Matrix* GetBoneOffsetTransformList();
	const AkI32* GetBoneHierarchyList();
	char** GetBoneName() { return _ppBoneName; }
	AkU32 GetBoneNum();
	AnimationClip_t* GetAnimationClip() { return _pAnimClip; }

private:
	void LoadMeshDataInfo(FILE* pFp, AkBool bForAnim);
	void LoadMaterialFileName(FILE* pFp);
	void LoadVerticesAndIndices(FILE* pFp, AkBool bForAnim);
	void LoadBoneOffsets(FILE* pFp);
	void LoadBoneHierarchy(FILE* pFp);
	void LoadBoneName(FILE* pFp);
	void LoadAnimationClip(FILE* pFp);

	void UpdateTangents();

private:
	MeshData_t* _pMeshData = nullptr;
	AkU32 _uMeshDataNum = 0;
	wchar_t _wcFileExtension[8] = {};
	const wchar_t* _wcBasePath = nullptr;

	Matrix* _pBoneOffsetMatrixList = nullptr;
	AkI32* _uBoneHierarchyList = nullptr;
	char** _ppBoneName = nullptr;
	AkU32 _uBoneNum = 0;
	AkBool _bIsAnim = AK_FALSE;

	AnimationClip_t* _pAnimClip = nullptr;
};
