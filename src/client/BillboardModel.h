#pragma once

#include "Model.h"

/*
===============
BillboardModel
===============
*/

class BillboardModels : public Model
{
public:
	BillboardModels(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum);
	BillboardModels(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	~BillboardModels();

	AkBool Initialize(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum);
	AkBool Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual void Render() override;

private:
	void CleanUp();

private:
	IBillboard* _pBillboard = nullptr;

	void* _pTreeTextureArray = nullptr;
};

