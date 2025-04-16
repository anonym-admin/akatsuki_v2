#pragma once

/*
==========
BaseObject
==========
*/

class Model;
class Transform;

class BaseObject
{
public:
	BaseObject();
	virtual ~BaseObject();

	virtual void Update() = 0;
	virtual void FinalUpdate() = 0;
	virtual void RenderDepthMap() = 0;
	virtual void RenderShadowMaps() = 0;
	virtual void Render() = 0;

	Model* GetModel() { return _pModel; }
	Transform* GetTransform() { return _pTransform; }

	List_t tLink = {};

protected:
	Model* CreateModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsSkinned);
	Model* CreateModel(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsSkinned);
	Transform* CreateTransform();

	Model* CreateBillboardModel(BillboardData_t* pBillboardData); // 삭제 진행 => Billboard 모델을 없애고 Actor 에서 자체적으로 생성.

private:
	void CleanUp();

protected:
	Model* _pModel = nullptr;
	Transform* _pTransform = nullptr;

public:
	wchar_t Name[_MAX_PATH] = {};
};

