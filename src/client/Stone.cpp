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

		// Asset Manager 에서 검색.
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

