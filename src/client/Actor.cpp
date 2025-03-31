#include "pch.h"
#include "Actor.h"
#include "Application.h"
#include "Camera.h"

/*
=========
Actor
=========
*/

Actor::~Actor()
{
	CleanUp();
}

AkBool Actor::Initialize(const wchar_t* wcScriptFile)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcScriptFile, L"rt");
	if (!fp) { __debugbreak(); }

	AkI32 iIsSkinned = 0;
	fwscanf_s(fp, L"%d\n", &iIsSkinned);
	if (!iIsSkinned)
	{
		wchar_t wcFilePath[_MAX_PATH] = {};

		fwscanf_s(fp, L"%s\n", wcFilePath, _MAX_PATH);
		wcscpy_s(Name, wcFilePath);
		wcscat_s(wcFilePath, L".mesh");

		// Asset Manager 에서 검색.
		Vector3 vAlbedo = Vector3(1.0f);
		Vector3 vEmissive = Vector3(0.0f);
		AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshData(wcFilePath);
		if (pMeshDataContainer)
		{
			_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_TRUE);
		}
		else
		{
			GAssetManager->AddMeshData(MESH_FILE_PATH, wcFilePath, 1.0f, AK_TRUE);

			pMeshDataContainer = GAssetManager->GetMeshData(wcFilePath);

			_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_TRUE);
		}
	}
	else
	{
		wchar_t wcFilePath[_MAX_PATH] = {};

		fwscanf_s(fp, L"%s\n", wcFilePath, _MAX_PATH);
		wcscpy_s(Name, wcFilePath);
		wcscat_s(wcFilePath, L".mesh");

		// Asset Manager 에서 검색.
		Vector3 vAlbedo = Vector3(1.0f);
		Vector3 vEmissive = Vector3(0.0f);
		AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshData(wcFilePath);
		if (pMeshDataContainer)
		{
			_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_FALSE);
		}
		else
		{
			GAssetManager->AddMeshData(MESH_FILE_PATH, wcFilePath, 1.0f, AK_FALSE);

			pMeshDataContainer = GAssetManager->GetMeshData(wcFilePath);

			_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_FALSE);
		}
	}

	// Collider 갯수 파싱 필요!!
	AkI32 iColliderNum = 0;
	fwscanf_s(fp, L"%d\n", &iColliderNum);
	_iEventColliderNum = iColliderNum - 1;
	if (_iEventColliderNum < 0)
	{
		_iEventColliderNum = 0;
	}

	for (AkI32 i = 0; i < iColliderNum; i++)
	{
		Collider* pCollider = nullptr;
		AkI32 iColliderType = 0;
		fwscanf_s(fp, L"%d\n", &iColliderType);

		switch (iColliderType)
		{
		case (AkI32)COLLIDER_TYPE::BOX:
			pCollider = CreateBoxCollider();
			break;
		case (AkI32)COLLIDER_TYPE::SPHERE:
			pCollider = CreateSphereCollider();
			break;
		case (AkI32)COLLIDER_TYPE::CAPSULE:
			pCollider = CreateCapsuleCollider();
			break;
		}

		Vector3 vScale = Vector3(1.0f);
		Vector3 vRotation = Vector3(0.0f);
		Vector3 vPosition = Vector3(0.0f);

		fwscanf_s(fp, L"%f %f %f\n", &vScale.x, &vScale.y, &vScale.z);
		fwscanf_s(fp, L"%f %f %f\n", &vRotation.x, &vRotation.y, &vRotation.z);
		fwscanf_s(fp, L"%f %f %f\n", &vPosition.x, &vPosition.y, &vPosition.z);

		pCollider->GetTransform()->SetScale(&vScale);
		pCollider->GetTransform()->SetRotation(&vRotation);
		pCollider->GetTransform()->SetPosition(&vPosition);

		if (0 == i)
		{
			_pCollider = pCollider;
		}
		else
		{
			_pEventCollider[i - 1] = pCollider;
		}
	}

	if (fp) { fclose(fp); }

	// Create Transform
	_pTransform = CreateTransform();

	return AK_TRUE;
}

Collider* Actor::CreateBoxCollider(const Vector3* pMin, const Vector3* pMax, const Vector3* pColor)
{
	Vector3 vMin = Vector3(-0.5f);
	Vector3 vMax = Vector3(0.5f);
	if (pMin)
		vMin = *pMin;
	if (pMax)
		vMax = *pMax;
	Collider* pCollider = new BoxCollider(this, &vMin, &vMax);
	return pCollider;
}

Collider* Actor::CreateSphereCollider(AkF32 fRadius, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	Collider* pCollider = new SphereCollider(this, fRadius, uStack, uSlice, pColor);
	return pCollider;
}

Collider* Actor::CreateCapsuleCollider(AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	Collider* pCollider = new CapsuleCollider(this, fRadius, fHeight, uStack, uSlice, pColor);
	return pCollider;
}

Collider* Actor::CreateSquareCollider()
{
	Collider* pCollider = new SquareCollider(this);
	return pCollider;
}

RigidBody* Actor::CreateRigidBody()
{
	RigidBody* pRigidBody = new RigidBody(this);
	return pRigidBody;
}

Gravity* Actor::CreateGravity()
{
	Gravity* pGravity = new Gravity(this);
	return pGravity;
}

Camera* Actor::CreateCamera(const Vector3* pPos, const Vector3* pYawPitchRoll)
{
	Vector3 vYawPitchRoll = Vector3(0.0f);
	if (!pYawPitchRoll)
		pYawPitchRoll = &vYawPitchRoll;
	Camera* pCam = new Camera(pPos, pYawPitchRoll);
	pCam->Mode = CAMERA_MODE::FREE;
	return pCam;
}

Camera* Actor::CreateCamera(AkF32 fDistance, AkF32 fHeight)
{
	Camera* pCam = new Camera(fDistance, fHeight);
	pCam->Mode = CAMERA_MODE::FOLLOW;
	return pCam;
}

Animation* Actor::CreateAnimation(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum)
{
	Animation* pAnim = new Animation(pMeshDataContainer, wcIdleClipName, uMaxClipNum);
	return pAnim;
}

void Actor::DestroyCollider()
{
	if (_pCollider)
	{
		delete _pCollider;
		_pCollider = nullptr;
	}

	for (AkI32 i = 0; i < _iEventColliderNum; i++)
	{
		if (_pEventCollider[i])
		{
			delete _pEventCollider[i];
			_pEventCollider[i] = nullptr;
		}
	}
}

void Actor::DesteoyRigidBody()
{
	if (_pRigidBody)
	{
		delete _pRigidBody;
		_pRigidBody = nullptr;
	}
}

void Actor::DestroyGravity()
{
	if (_pGravity)
	{
		delete _pGravity;
		_pGravity = nullptr;
	}
}

void Actor::DestroyCamera()
{
	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}
}

void Actor::DestroyAnimation()
{
	if (_pAnimation)
	{
		delete _pAnimation;
		_pAnimation = nullptr;
	}
}

void Actor::BindAnimation(Animation* pAnim)
{
	_pAnimation = pAnim;
	if (_pModel)
	{
		((SkinnedModel*)_pModel)->BindAnimation(pAnim);
	}
}

void Actor::UnBindAnimation()
{
	_pAnimation = nullptr;
}

void Actor::SetWeapon(Weapon* pWeapon)
{
	BindWeapon = AK_TRUE;
	_pWeapon = pWeapon;
}

void Actor::CleanUp()
{
	AkU32 uRefCount = _uInstanceCount - 1;
	if (uRefCount)
	{
		return;
	}

	DestroyCollider();
	DesteoyRigidBody();
	DestroyGravity();
	DestroyCamera();
	DestroyAnimation();
}





