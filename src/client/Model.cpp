#include "pch.h"
#include "Model.h"
#include "AssetManager.h"

Model::Model(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	if (!Initialize(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive))
	{
		__debugbreak();
	}
}

Model::~Model()
{
	CleanUp();
}

AkBool Model::Initialize(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	CreateMeshObject(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum);
	CreateMaterial(pAlbedo, fMetallic, fRoughness, pEmissive);

	return AK_TRUE;
}

void Model::Render()
{
	GRenderer->RenderBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::RenderNormal()
{
	GRenderer->RenderNormalOfBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::RenderShadow()
{
	GRenderer->RenderShadowOfBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::UpdateWorldRow(const Matrix* pWorldRow)
{
	_mWorldRow = *pWorldRow;
}

void Model::SetWireFrame(AkBool bDrawWire)
{
	if (bDrawWire)
		_pMeshObj->EnableWireFrame();
	else
		_pMeshObj->DisableWireFrame();
}

void Model::CleanUp()
{
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}
}

void Model::CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	_pMeshObj = GRenderer->CreateBasicMeshObject();
	_pMeshObj->CreateMeshBuffers(pMeshData, uMeshDataNum);
}

void Model::CreateMaterial(const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	_pMeshObj->UpdateMaterialBuffers(pAlbedo, fMetallic, fRoughness, pEmissive);
}