#pragma once

#include "Model.h"

class Animation;

class SkinnedModel : public Model
{
public:
	SkinnedModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	~SkinnedModel();

	virtual void Render();
	virtual void RenderNormal();
	virtual void RenderShadow();
	void BindAnimation(Animation* pAnim) { _pAnim = pAnim; }
	Animation* GetAnimation() { return _pAnim; }
	AkBool PlayAnimation(const wchar_t* wcClipName, AkBool bInPlace);

private:
	virtual void CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum);

private:
	Animation* _pAnim = nullptr;
};

