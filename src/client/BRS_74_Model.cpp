#include "pch.h"
#include "BRS_74_Model.h"
#include "Application.h"
#include "AssetManager.h"

UBRS_74_Model::UBRS_74_Model()
{
}

UBRS_74_Model::~UBRS_74_Model()
{
	CleanUp();
}

AkBool UBRS_74_Model::Initialize(UApplication* pApp)
{
	if (!UModel::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pApp = pApp;
	_pRenderer = pApp->GetRenderer();

	UAssetManager* pAssetManager = _pApp->GetAssetManager();

	if (CreateStaticMeshObject())
	{
		// Mesh Data.
		AssetMeshDataContainer_t* pMeshDataContainer = pAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_BRS_74);
		MeshData_t* pBRS_74_MeshData = pMeshDataContainer->pMeshData;
		AkU32 uBRS_74_MeshDataNum = pMeshDataContainer->uMeshDataNum;

		// TODO!!
		// Change the model scale ??

		CreateMeshBuffersForDraw(pBRS_74_MeshData, uBRS_74_MeshDataNum);
		SetMinMaxVertexPosition(pBRS_74_MeshData, uBRS_74_MeshDataNum);

		// Material.
		IMeshObject* pMeshObj = GetRenderObject();
		Vector3 vAlbedoFactor = Vector3(1.0f);
		AkF32 fMetallicFactor = 0.0f;
		AkF32 fRoughnessFactor = 1.0f;
		Vector3 vEmissiveFactor = Vector3(0.0f);
		pMeshObj->UpdateMaterialBuffers(&vAlbedoFactor, fMetallicFactor, fRoughnessFactor, &vEmissiveFactor);

		Matrix mWorldRow = Matrix();
		// Ref 0 
		CreateContextTable(&mWorldRow, nullptr);

		pAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::ASSET_MESH_DATA_TYPE_BRS_74);
	}
	else
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

void UBRS_74_Model::Update(const AkF32 fDeltaTime)
{
}

void UBRS_74_Model::RenderShadow()
{
	IMeshObject* pMeshObj = GetRenderObject();
	const Matrix* pWorldMat = GetWorldMatrix();

	_pRenderer->RenderShadowOfBasicMeshObject(pMeshObj, pWorldMat);
}

void UBRS_74_Model::Render()
{
	IMeshObject* pMeshObj = GetRenderObject();
	const Matrix* pWorldMat = GetWorldMatrix();

	_pRenderer->RenderBasicMeshObject(pMeshObj, pWorldMat);
}

void UBRS_74_Model::RenderNormal()
{
	IMeshObject* pMeshObj = GetRenderObject();
	const Matrix* pWorldMat = GetWorldMatrix();

	_pRenderer->RenderNormalOfBasicMeshObject(pMeshObj, pWorldMat);
}

void UBRS_74_Model::Release()
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
}

void UBRS_74_Model::SetRelativeRotationYaw(AkF32 fRot)
{
	_vRelativeRot.x = fRot;
}

void UBRS_74_Model::SetRelativeRotationPitch(AkF32 fRot)
{
	_vRelativeRot.y = fRot;
}

void UBRS_74_Model::SetRelativeRotationRoll(AkF32 fRot)
{
	_vRelativeRot.z = fRot;
}

void UBRS_74_Model::SetRelativePosition(Vector3 vPos)
{
	_vRelativePos = vPos;
}

void UBRS_74_Model::SetAnimTransform(Matrix* pMat)
{
	_pAnimTransform = pMat;
}

void UBRS_74_Model::GetMinMaxPosition(Vector3* pOutMin, Vector3* pOutMax)
{
	*pOutMin = _vMinVertPos;
	*pOutMax = _vMaxVertPos;
}

void UBRS_74_Model::CleanUp()
{
}

void UBRS_74_Model::SetMinMaxVertexPosition(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	Vector3 vMin = Vector3(AK_MAX_F32);
	Vector3 vMax = Vector3(-AK_MAX_F32);
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < pMeshData[i].uVerticeNum; j++)
		{
			vMin.x = min(vMin.x, pMeshData[i].pVertices[j].vPosition.x);
			vMin.y = min(vMin.y, pMeshData[i].pVertices[j].vPosition.y);
			vMin.z = min(vMin.z, pMeshData[i].pVertices[j].vPosition.z);

			vMax.x = max(vMax.x, pMeshData[i].pVertices[j].vPosition.x);
			vMax.y = max(vMax.y, pMeshData[i].pVertices[j].vPosition.y);
			vMax.z = max(vMax.z, pMeshData[i].pVertices[j].vPosition.z);
		}
	}

	_vMinVertPos = vMin;
	_vMaxVertPos = vMax;
}
