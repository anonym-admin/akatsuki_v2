#pragma once

#include "Model.h"

class TreeModel : public Model
{
public:
	TreeModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	TreeModel(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	~TreeModel();

	AkBool Initialize();
	virtual void Render() override;
	virtual void RenderGUI() override;

	AkF32 GetWindTrunk() { return _fWindTrunk; }
	AkF32 GetWindLeaves() { return _fWindLeaves; }
	void SetWindTrunk(AkF32 fWindTrunk) { _fWindTrunk = fWindTrunk; }
	void SetWindLeaves(AkF32 fWindLeaves) { _fWindLeaves = fWindLeaves; }

private:
	AkF32 _fWindTrunk = 0.0f;
	AkF32 _fWindLeaves = 0.0f;
};

