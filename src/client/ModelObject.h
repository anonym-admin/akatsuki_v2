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
	static const AkU32 MAX_EVENT_COLLIDER_NUM = 8;

	ModelObject() = default;
	ModelObject(const ModelObject& rOrigin);
	ModelObject(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsAnim);
	ModelObject(const wchar_t* wcFilename, AkBool bIsAnim);
	ModelObject(const wchar_t* wcScript);
	virtual ~ModelObject();

	AkBool Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsAnim);
	AkBool Initialize(const wchar_t* wcFilename, AkBool bIsAnim);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	void RenderGUI();
	void SetEditMode(AkBool bIsEditMode) { _bEditMode = bIsEditMode; }

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	virtual ModelObject* Clone() override;

private:
	void CleanUp();

private:
	AkBool _bUseGizmo = AK_FALSE;
	AkBool _bEditMode = AK_FALSE;
};

