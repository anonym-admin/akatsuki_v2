#pragma once

/*
========
Model
========
*/

class Model
{
public:
	Model() = default;
	Model(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	Model(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual ~Model();

	virtual AkBool Initialize(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual AkBool Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual void Render();
	virtual void RenderNormal();
	virtual void RenderShadow();
	virtual void RenderGUI();
	void UpdateWorldRow(Matrix* pWorldRow);
	Matrix GetWorldRow() { return _mWorldRow; }
	void SetWireFrame(AkBool bDrawWire);

	AkBool IsPick() { return _bUseGizmo; }

private:
	void CleanUp();
	virtual void CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum);

protected:
	virtual void CreateMaterial(const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);

protected:
	IMeshObject* _pMeshObj = nullptr;
	Matrix _mWorldRow = Matrix();
	AkBool _bUseGizmo = AK_FALSE;
	
public:
	wchar_t Name[_MAX_PATH] = {};
};

