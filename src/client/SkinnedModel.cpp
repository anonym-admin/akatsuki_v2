#include "pch.h"
#include "SkinnedModel.h"
#include "AssetManager.h"
#include "Animation.h"

SkinnedModel::SkinnedModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	if (!Initialize(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive))
	{
		__debugbreak();
	}
}

SkinnedModel::~SkinnedModel()
{

}

void SkinnedModel::Render()
{
	Matrix pIdentity[96] = {};
	const Matrix* pBoneTransform = _pAnim ? _pAnim->GetFinalTransforms() : pIdentity;

	GRenderer->RenderSkinnedMeshObject(_pMeshObj, &_mWorldRow, pBoneTransform);
}

void SkinnedModel::RenderNormal()
{
	Matrix pIdentity[96] = {};
	const Matrix* pBoneTransform = _pAnim ? _pAnim->GetFinalTransforms() : pIdentity;

	GRenderer->RenderNormalOfSkinnedMeshObject(_pMeshObj, &_mWorldRow, pBoneTransform);
}

void SkinnedModel::RenderShadow()
{
	Matrix pIdentity[96] = {};
	const Matrix* pBoneTransform = _pAnim ? _pAnim->GetFinalTransforms() : pIdentity;

	GRenderer->RenderShadowOfSkinnedMeshObject(_pMeshObj, &_mWorldRow, pBoneTransform);
}

AkBool SkinnedModel::PlayAnimation(const wchar_t* wcClipName, AkBool bInPlace)
{
	return _pAnim->PlayAnimation(wcClipName, bInPlace);
}

void SkinnedModel::CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	_pMeshObj = GRenderer->CreateSkinnedMeshObject();
	_pMeshObj->CreateMeshBuffers(pMeshData, uMeshDataNum);
}

