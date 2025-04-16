#pragma once

#include "Model.h"

class StoneModel : public Model
{
public:
	StoneModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	StoneModel(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	~StoneModel();

	AkBool Initialize();
	virtual void Render() override;
	virtual void RenderGUI() override;

	AkF32 GetHeightScale() { return _fHeightScale; }
	void SetHeightScale(AkF32 fHeightScale) { _fHeightScale = fHeightScale; }

private:
	AkF32 _fHeightScale = 0.0f;
};

