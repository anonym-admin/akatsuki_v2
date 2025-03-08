#include "pch.h"
#include "Editor.h"
#include "SkinnedModel.h"
#include "Camera.h"
#include "Transform.h"
#include "Animation.h"

/*
=========
Editor
=========
*/

Model* Editor::CreateModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsSkinned)
{
	Model* pModel = nullptr;
	if (bIsSkinned)
	{
		pModel = new SkinnedModel(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive);
	}
	else
	{
		pModel = new Model(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive);
	}
	return pModel;
}

Camera* Editor::CreateCamera()
{
	Vector3 vCamPos = Vector3(0.0f, 0.0f, -2.0f);
	Vector3 vYawPitchRoll = Vector3(0.0f);
	Camera* pCam = new Camera(&vCamPos, &vYawPitchRoll);
	pCam->Mode = CAMERA_MODE::EDITOR;
	return pCam;
}

Transform* Editor::CreateTransform()
{
	Transform* pTransform = new Transform;
	return pTransform;
}

Animation* Editor::CreateAnimation(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum)
{
	Animation* pAnim = new Animation(pMeshDataContainer, wcIdleClipName, uMaxClipNum);
	return pAnim;
}
