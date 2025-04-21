#include "pch.h"
#include "GrassModel.h"

#include <random>

GrassModel::GrassModel(GrassInfo* pInfo)
{
	if (!Initialize(pInfo))
	{
		__debugbreak();
	}
}

GrassModel::~GrassModel()
{
	CleanUp();
}

AkBool GrassModel::Initialize(GrassInfo* pInfo)
{
	_GrassInfo = *pInfo;

	// Create Grass Model.
	AkU32 uMeshDataNum = 0;
	MeshData_t* pGrassMeshData = GeometryGenerator::MakeGrass(&uMeshDataNum);
	_pGrassObj = GRenderer->CreateBasicMeshObject();
	_pGrassObj->CreateMeshBuffers(pGrassMeshData, uMeshDataNum);

	// Instance »ý¼º
	std::mt19937 gen(0);
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	VertexInstance_t* pInstance = new VertexInstance_t[_GrassInfo.uInstanceCount];
	for (AkU32 i = 0; i < _GrassInfo.uInstanceCount; i++)
	{
		const AkF32 fLengthScale = dist(gen) * 0.7f + 0.3f;
		const AkF32 fWidthScale = dist(gen) * 0.5f + 0.5f;
		const Vector3 vPosition = Vector3(dist(gen) * 2.0f - 1.0f, 0.0f, dist(gen) * 2.0f - 1.0f) * 0.5f;
		const AkF32 fAngle = dist(gen) * DirectX::XM_PI;
		const AkF32 fSlope = (dist(gen) - 0.5f) * 2.0f * DirectX::XM_PI * 0.2f;

		VertexInstance_t Inst;
		Inst.mInstanceWorld = Matrix::CreateRotationX(fSlope) * Matrix::CreateRotationY(fAngle) *
			Matrix::CreateScale(fWidthScale, fLengthScale, 1.0f) * Matrix::CreateTranslation(vPosition);

		for (AkU32 j = 0; j < uMeshDataNum; j++)
		{
			for (AkU32 k = 0; k < pGrassMeshData[j].uVerticeNum; k++)
			{
				Vector3 vTemp = Vector3::Transform(pGrassMeshData[j].pVertices[k].vPosition, Inst.mInstanceWorld);

				_vMin.x = min(_vMin.x, vTemp.x);
				_vMin.y = min(_vMin.y, vTemp.y);
				_vMin.z = min(_vMin.z, vTemp.z);

				_vMax.x = max(_vMax.x, vTemp.x);
				_vMax.y = max(_vMax.y, vTemp.y);
				_vMax.z = max(_vMax.z, vTemp.z);
			}
		}

		Inst.mInstanceWorld = Inst.mInstanceWorld.Transpose();
		Inst.fWindStrength = _GrassInfo.fWindStrength;

		pInstance[i] = Inst;
	}

	_pGrassObj->CreateInstanceBuffers(pInstance, _GrassInfo.uInstanceCount);
	_pGrassObj->SetIBLStrength(0.15f);

	GeometryGenerator::DestroyGeometry(pGrassMeshData, uMeshDataNum);

	if(pInstance)
	{
		delete[] pInstance;
		pInstance = nullptr;
	}

	return AK_TRUE;
}

void GrassModel::Render()
{
	GRenderer->RenderBasicMeshObject(_pGrassObj, &_mWorldRow);
}

void GrassModel::RenderGUI()
{
	// TODO:
}

void GrassModel::GetMinMax(Vector3* pOutMin, Vector3* pOutMax)
{
	*pOutMin = Vector3::Transform(_vMin, _mWorldRow);
	*pOutMax = Vector3::Transform(_vMax, _mWorldRow);
}

void GrassModel::CleanUp()
{
	if (_pGrassObj)
	{
		_pGrassObj->Release();
		_pGrassObj = nullptr;
	}
}
