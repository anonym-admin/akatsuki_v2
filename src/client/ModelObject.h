#pragma once

#include "Spawn.h"

/*
=================
Model Object
=================
*/

class ModelObject : public Spawn
{
public:
	ModelObject() = default;
	ModelObject(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	ModelObject(const wchar_t* wcFilename);
	virtual ~ModelObject();

	AkBool Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	AkBool Initialize(const wchar_t* wcFilename);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	virtual ModelObject* Clone() override;

	void CreateCube(const Vector3* pExtent, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, const wchar_t** ppTexFimenames);

private:
	void CleanUp();
};

