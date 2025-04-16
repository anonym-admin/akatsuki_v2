#pragma once

/*
========
Model
========
*/

/*
===============================================
1. 동일한 모델 객체를 게임오브젝트에서 공유할수 있다.
2. Ref Count 로 관리 필요
===============================================
*/

class Model
{
public:
	Model() = default;
	Model(const Model& Other);
	Model(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	Model(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual ~Model();

	virtual AkBool Initialize(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual AkBool Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual void RenderShadowMaps();
	virtual void RenderDepthMap();
	virtual void Render();
	virtual void RenderNormals();
	virtual void RenderGUI();

	Matrix GetWorldRow() { return _mWorldRow; }
	void UpdateWorldRow(Matrix* pWorldRow);
	void SetWireFrame(AkBool bDrawWire);
	void SetTextures(void* pAlbedo, void* pEmissve, void* pHeight, void* pNormal, void* pMetallic, void* pRoughness, void* pAO);

	AkBool IsPick() { return _bUseGizmo; }
	void ReleasePick() { _bUseGizmo = AK_FALSE; }

	void operator=(const Model& Other);
	
	AkU32 AddRef();
	AkU32 Release();

private:
	void CleanUp();
	virtual void CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum);

protected:
	virtual void CreateMaterial(const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);

protected:
	AkU32 _uRefCount = 1;
	IMeshObject* _pMeshObj = nullptr;
	Matrix _mWorldRow = Matrix();
	AkBool _bUseGizmo = AK_FALSE;
	
public:
	wchar_t Name[_MAX_PATH] = {};
};

