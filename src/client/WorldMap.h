#pragma once

#include "Actor.h"

class UApplication;
struct KDTreeNode_t;

class UWorldMapContainer : public UActor
{
public:
	static const AkU32 MESH_DATA_CHUNK_COUNT = 8192;
	static const AkU32 MAX_MESH_OBJ_LIST_COUNT = 1024;

	UWorldMapContainer();
	~UWorldMapContainer();

	virtual AkBool Initialize(UApplication* pApp);
	virtual void Update(const AkF32 fDeltaTime);
	virtual void FinalUpdate(const AkF32 fDeltaTime);
	virtual void RenderShadow();
	virtual void Render();

	virtual void OnCollision(UCollider* pOther);
	virtual void OnCollisionEnter(UCollider* pOther);
	virtual void OnCollisionExit(UCollider* pOther);

	void BindMeshData(MeshData_t* pMeshData, AkU32 uMeshDataNum);
	void BindMeshObj(IMeshObject* pMeshObj, AkU32 uMeshObjNum, const Matrix* pWorldRows);
	void Build(AkBool bUseKDTree);

	UCollider** GetColliderList() { return _ppColliderList; }
	AkU32 GetColliderNum() { return _uColliderNum; }
	KDTreeNode_t* GetKDTreeNode() { return _pKDTreeNode; }
	void SetDrawKDTreeNodeFlag(AkBool bDrawKDTreeNode) { _bDrawKDTreeNode = bDrawKDTreeNode; }
	void SetMeshObjNum(AkU32 uIndex, AkU32 uMeshObjNum) { _uMeshObjNum[uIndex] = uMeshObjNum; }
	void SetWorldRowList(AkU32 uIndex, const Matrix* pWorldRows) { _pWorldRows[uIndex] = pWorldRows; }

	AkBool UseOptimize() { return _bUseKDTree; }

private:
	virtual void CleanUp();

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;
	IMeshObject* _pMeshObj[MAX_MESH_OBJ_LIST_COUNT] = {};
	AkU32 _uMeshObjNum[MAX_MESH_OBJ_LIST_COUNT] = {};
	AkU32 _uMsshObjListNum = 0;
	const Matrix* _pWorldRows[MAX_MESH_OBJ_LIST_COUNT] = {};

	AkU32 _uMeshDataChunkNum = 0;
	MeshData_t* _ppMeshDataChunk[MESH_DATA_CHUNK_COUNT] = {};
	AkU32 _pMeshDataNum[MESH_DATA_CHUNK_COUNT] = {};
	
	UCollider** _ppColliderList = nullptr;
	AkU32 _uColliderNum = 0;
	AkU32 _uInitColliderID = 0;

	// For KDTree.
	IMeshObject* _pKDTreeBoxObj = nullptr;
	KDTreeNode_t* _pKDTreeNode = nullptr;
	AkBool _bUseKDTree = AK_FALSE;
	AkBool _bDrawKDTreeNode = AK_FALSE;


public:
	AkBool* _pDrawTable = nullptr;
};

