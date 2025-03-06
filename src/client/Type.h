/*
==================
Asset Container
==================
*/

struct AssetMeshDataContainer_t
{
	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 0;
	Matrix mDefaultMat = Matrix();
	const Matrix* pBoneOffsetMatList = nullptr;
	const AkI32* pBoneHierarchyList = nullptr;
	AkU32 uBoneNum = 0;
};

struct AssetTextureContainer_t
{
	void* pTexHandle = nullptr;
};