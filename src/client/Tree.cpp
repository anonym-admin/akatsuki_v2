#include "pch.h"
#include "Tree.h"
#include "TreeModel.h"

Tree::Tree(const wchar_t* wcScriptFile)
{
	if (!Initialize(wcScriptFile))
	{
		__debugbreak();
	}
}

Tree::Tree(const Tree& Other)
	: ModelObject(Other)
{
}

Tree::~Tree()
{
}

AkBool Tree::Initialize(const wchar_t* wcScriptFile)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcScriptFile, L"rt");
	if (!fp) { __debugbreak(); }

	AssetMeshDataContainer_t* pMeshDataContainer = nullptr;
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
		pMeshDataContainer = GAssetManager->GetMeshData(wcFilePath);
		if (pMeshDataContainer)
		{
			_pModel = new TreeModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive);
		}
		else
		{
			GAssetManager->AddMeshData(MESH_FILE_PATH, wcFilePath, 1.0f, AK_FALSE);

			pMeshDataContainer = GAssetManager->GetMeshData(wcFilePath);

			_pModel = new TreeModel(pMeshDataContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive);
		}
	}

	if (fp) { fclose(fp); }

	// Create Transform
	_pTransform = CreateTransform();

	// Create Culling Collider.
	Vector3 vMin = Vector3(0.0f);
	Vector3 vMax = Vector3(0.0f);
	CalcColliderMinMax(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum, &vMin, &vMax);
	_pCullingCollider = CreateBoxCollider(&vMin, &vMax);

	return AK_TRUE;
}

void Tree::RenderGUI()
{
	ModelObject::RenderGUI();

	_pModel->RenderGUI();
}
