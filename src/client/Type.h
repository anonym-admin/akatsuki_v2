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
	const Matrix* pBoneOffsetMatrixList = nullptr;
	const AkI32* pBoneHierarchyList = nullptr;
	AkU32 uBoneNum = 0;
};

struct AssetTextureContainer_t
{
	void* pTexHandle = nullptr;
};

/*
======================
Event Manager Handle
======================
*/

struct EventCreateGameObjectParam_t
{
	GAME_OBJECT_GROUP_TYPE eType = {};
	const wchar_t* Name = nullptr;
};

struct EventSceneAndEditorChangeParam_t
{
	SCENE_TYPE eBeforeScene = {};
	SCENE_TYPE eAfterScene = {};
	EDITOR_TYPE eBeforeEditor = {};
	EDITOR_TYPE eAfterEditor = {};
};

struct EventHandle_t
{
	EVENT_TYPE eEventType = {};
	void* pObj = nullptr;

	union
	{
		EventCreateGameObjectParam_t tCreateGameObjParam;
		EventSceneAndEditorChangeParam_t tSceneAndEditorChangeParam;
	};
};
