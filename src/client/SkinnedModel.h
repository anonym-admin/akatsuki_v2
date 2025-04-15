#pragma once

#include "Model.h"

/*
=============
SkinnedModel
=============
*/

class Animation;

class SkinnedModel : public Model
{
public:
	SkinnedModel(const SkinnedModel& Other);
	SkinnedModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	SkinnedModel(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	~SkinnedModel();

	virtual void RenderDepthMap() override;
	virtual void RenderShadowMaps() override;
	virtual void Render() override;
	virtual void RenderNormals() override;

	void BindAnimation(Animation* pAnim) { _pAnim = pAnim; }
	void UnBindAnimation() { _pAnim = nullptr; }
	Animation* GetAnimation() { return _pAnim; }

	void operator=(const SkinnedModel& Other);

private:
	virtual void CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum);

private:
	Animation* _pAnim = nullptr;

	Matrix _pIdentity[96] = {}; // For Bone Transform => 애니매이션이 바운딩 되지 않았을때 사용.
};


