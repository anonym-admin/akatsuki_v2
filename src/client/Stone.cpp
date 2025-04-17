#include "pch.h"
#include "Stone.h"
#include "StoneModel.h"

Stone::Stone(const wchar_t* wcScriptFile)
{
	if (!Initialize(wcScriptFile))
	{
		__debugbreak();
	}
}

Stone::Stone(const Stone& Other)
	: ModelObject(Other)
{
}

Stone::~Stone()
{
}

AkBool Stone::Initialize(const wchar_t* wcScriptFile)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcScriptFile, L"rt");
	if (!fp) { __debugbreak(); }

	AkI32 iIsSkinned = 0;
	fwscanf_s(fp, L"%d\n", &iIsSkinned);
	if (!iIsSkinned)
	{
		__debugbreak();
	}
	else
	{
		wchar_t wcFilePath[_MAX_PATH] = {};

		fwscanf_s(fp, L"%s\n", wcFilePath, _MAX_PATH);
		wcscpy_s(Name, wcFilePath);
		wcscat_s(wcFilePath, L".mesh");

		// Asset Manager ¿¡¼­ °Ë»ö.
		Vector3 vAlbedo = Vector3(1.0f);
		Vector3 vEmissive = Vector3(0.0f);
		AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshData(wcFilePath);
		if (pMeshDataContainer)
		{
			_pModel = new StoneModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive);
		}
		else
		{
			GAssetManager->AddMeshData(MESH_FILE_PATH, wcFilePath, 1.0f, AK_FALSE);

			pMeshDataContainer = GAssetManager->GetMeshData(wcFilePath);

			_pModel = new StoneModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive);
		}
	}

	// IBL Strength ÆÄ½Ì.
	AkF32 fIBLStrength = 0.0f;
	fwscanf_s(fp, L"%f\n", &fIBLStrength);
	_pModel->SetIBLStrength(fIBLStrength);

	// Collider °¹¼ö ÆÄ½Ì ÇÊ¿ä!!
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

void Stone::RenderGUI()
{
	ModelObject::RenderGUI();

	_pModel->RenderGUI();
}

