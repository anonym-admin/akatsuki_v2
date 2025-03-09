#pragma once

class Model;
class Transform;

class BaseObject
{
public:
	BaseObject();
	virtual ~BaseObject();

	virtual void Update() = 0;
	virtual void FinalUpdate() = 0;
	virtual void Render() = 0;
	virtual void RenderShadow() = 0;

	Model* GetModel() { return _pModel; }
	Transform* GetTransform() { return _pTransform; }

	List_t tLink = {};

protected:
	Model* CreateModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsSkinned);
	Model* CreateModel(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsSkinned);
	Transform* CreateTransform();

private:
	void CleanUp();

protected:
	AkU32 _uInstanceCount = 0;
	Model* _pModel = nullptr;
	Transform* _pTransform = nullptr;

public:
	const wchar_t* Name = nullptr;
};

