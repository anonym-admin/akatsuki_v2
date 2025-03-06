#include "pch.h"
#include "GeometryGenerator.h"
#include "ModelImporter.h"
#include "Animation.h"
#include "Application.h"

/*
===================
Geometry Generator
===================
*/

void UGeometryGenerator::NormalizeMeshData(MeshData_t* pMeshData, AkU32 uMeshDataNum, AkF32 fScaleLength)
{
	Vector3 vMax = Vector3(-1000.0f, -1000.0f, -1000.0f);
	Vector3 vMin = Vector3(1000.0f, 1000.0f, 1000.0f);

	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		MeshData_t tMeshData = pMeshData[i];
		for (AkU32 j = 0; j < tMeshData.uVerticeNum; j++)
		{
			if (!tMeshData.pSkinnedVertices)
			{
				Vertex_t v = tMeshData.pVertices[j];
				vMax.x = DirectX::XMMax(v.vPosition.x, vMax.x);
				vMin.x = DirectX::XMMin(v.vPosition.x, vMin.x);
				vMax.y = DirectX::XMMax(v.vPosition.y, vMax.y);
				vMin.y = DirectX::XMMin(v.vPosition.y, vMin.y);
				vMax.z = DirectX::XMMax(v.vPosition.z, vMax.z);
				vMin.z = DirectX::XMMin(v.vPosition.z, vMin.z);
			}
			else
			{
				SkinnedVertex_t v = tMeshData.pSkinnedVertices[j];
				vMax.x = DirectX::XMMax(v.vPosition.x, vMax.x);
				vMin.x = DirectX::XMMin(v.vPosition.x, vMin.x);
				vMax.y = DirectX::XMMax(v.vPosition.y, vMax.y);
				vMin.y = DirectX::XMMin(v.vPosition.y, vMin.y);
				vMax.z = DirectX::XMMax(v.vPosition.z, vMax.z);
				vMin.z = DirectX::XMMin(v.vPosition.z, vMin.z);
			}
		}
	}

	AkF32 dx = vMax.x - vMin.x, dy = vMax.y - vMin.y, dz = vMax.z - vMin.z;
	AkF32 scale = fScaleLength / DirectX::XMMax(DirectX::XMMax(dx, dy), dz);
	Vector3 translation = -(vMax + vMin) * 0.5f;

	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		MeshData_t tMeshData = pMeshData[i];
		for (AkU32 j = 0; j < tMeshData.uVerticeNum; j++)
		{
			if (!tMeshData.pSkinnedVertices)
			{
				pMeshData[i].pVertices[j].vPosition = (pMeshData[i].pVertices[j].vPosition + translation) * scale;
			}
			else
			{
				pMeshData[i].pSkinnedVertices[j].vPosition = (pMeshData[i].pSkinnedVertices[j].vPosition + translation) * scale;
			}
		}
	}
}

void UGeometryGenerator::NormalizeMeshData(MeshData_t* pMeshData, AkU32 uMeshDataNum, const AkF32 fScaleLength, AkBool bIsAnim, Matrix* pDefaultMatrix)
{
	Vector3 vMax = Vector3(-1000.0f, -1000.0f, -1000.0f);
	Vector3 vMin = Vector3(1000.0f, 1000.0f, 1000.0f);

	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		MeshData_t tMeshData = pMeshData[i];
		for (AkU32 j = 0; j < tMeshData.uVerticeNum; j++)
		{
			if (!bIsAnim)
			{
				Vertex_t v = tMeshData.pVertices[j];
				vMax.x = DirectX::XMMax(v.vPosition.x, vMax.x);
				vMin.x = DirectX::XMMin(v.vPosition.x, vMin.x);
				vMax.y = DirectX::XMMax(v.vPosition.y, vMax.y);
				vMin.y = DirectX::XMMin(v.vPosition.y, vMin.y);
				vMax.z = DirectX::XMMax(v.vPosition.z, vMax.z);
				vMin.z = DirectX::XMMin(v.vPosition.z, vMin.z);
			}
			else
			{
				SkinnedVertex_t v = tMeshData.pSkinnedVertices[j];
				vMax.x = DirectX::XMMax(v.vPosition.x, vMax.x);
				vMin.x = DirectX::XMMin(v.vPosition.x, vMin.x);
				vMax.y = DirectX::XMMax(v.vPosition.y, vMax.y);
				vMin.y = DirectX::XMMin(v.vPosition.y, vMin.y);
				vMax.z = DirectX::XMMax(v.vPosition.z, vMax.z);
				vMin.z = DirectX::XMMin(v.vPosition.z, vMin.z);
			}
		}
	}

	AkF32 dx = vMax.x - vMin.x, dy = vMax.y - vMin.y, dz = vMax.z - vMin.z;
	AkF32 scale = fScaleLength / DirectX::XMMax(DirectX::XMMax(dx, dy), dz);
	Vector3 translation = -(vMax + vMin) * 0.5f;

	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		MeshData_t tMeshData = pMeshData[i];
		for (AkU32 j = 0; j < tMeshData.uVerticeNum; j++)
		{
			if (!bIsAnim)
			{
				pMeshData[i].pVertices[j].vPosition = (pMeshData[i].pVertices[j].vPosition + translation) * scale;
			}
			else
			{
				pMeshData[i].pSkinnedVertices[j].vPosition = (pMeshData[i].pSkinnedVertices[j].vPosition + translation) * scale;
			}
		}
	}

	if (bIsAnim)
	{
		*pDefaultMatrix = Matrix::CreateTranslation(translation) * Matrix::CreateScale(scale);
	}
}

MeshData_t* UGeometryGenerator::MakeTriangle(AkU32* pMeshDataNum)
{
	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 1;
	pMeshData = new MeshData_t[uMeshDataNum];

	pMeshData->uVerticeNum = 3;
	pMeshData->uIndicesNum = 3;
	pMeshData->pVertices = new Vertex_t[pMeshData->uVerticeNum];
	pMeshData->pIndices = new AkU32[pMeshData->uIndicesNum];

	pMeshData->pVertices[0].vPosition = Vector3(-0.5f, -0.5f, 0.0f);
	pMeshData->pVertices[1].vPosition = Vector3(0.0f, 0.5f, 0.0f);
	pMeshData->pVertices[2].vPosition = Vector3(0.5f, -0.5f, 0.0f);
	pMeshData->pVertices[0].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData->pVertices[1].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData->pVertices[2].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData->pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData->pVertices[1].vTexCoord = Vector2(0.5f, 0.5f);
	pMeshData->pVertices[2].vTexCoord = Vector2(1.0f, 1.0f);

	pMeshData->pIndices[0] = 0;
	pMeshData->pIndices[1] = 1;
	pMeshData->pIndices[2] = 2;

	*pMeshDataNum = uMeshDataNum;

	return pMeshData;
}

MeshData_t* UGeometryGenerator::MakeSquare(AkU32* pMeshDataNum, const AkF32 fScale, const Vector2* pTexScale)
{
	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 1;
	pMeshData = new MeshData_t[uMeshDataNum];

	pMeshData->uVerticeNum = 4;
	pMeshData->uIndicesNum = 6;
	pMeshData->pVertices = new Vertex_t[pMeshData->uVerticeNum];
	pMeshData->pIndices = new AkU32[pMeshData->uIndicesNum];

	pMeshData->pVertices[0].vPosition = Vector3(-1.0f, 1.0f, 0.0f) * fScale * 0.5f;
	pMeshData->pVertices[1].vPosition = Vector3(1.0f, 1.0f, 0.0f) * fScale * 0.5f;
	pMeshData->pVertices[2].vPosition = Vector3(1.0f, -1.0f, 0.0f) * fScale * 0.5f;
	pMeshData->pVertices[3].vPosition = Vector3(-1.0f, -1.0f, 0.0f) * fScale * 0.5f;
	pMeshData->pVertices[0].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData->pVertices[1].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData->pVertices[2].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData->pVertices[3].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData->pVertices[0].vTexCoord = Vector2(0.0f, 0.0f) * (*pTexScale);
	pMeshData->pVertices[1].vTexCoord = Vector2(1.0f, 0.0f) * (*pTexScale);
	pMeshData->pVertices[2].vTexCoord = Vector2(1.0f, 1.0f) * (*pTexScale);
	pMeshData->pVertices[3].vTexCoord = Vector2(0.0f, 1.0f) * (*pTexScale);

	pMeshData->pIndices[0] = 0;
	pMeshData->pIndices[1] = 1;
	pMeshData->pIndices[2] = 2;
	pMeshData->pIndices[3] = 0;
	pMeshData->pIndices[4] = 2;
	pMeshData->pIndices[5] = 3;

	*pMeshDataNum = uMeshDataNum;

	return pMeshData;
}

MeshData_t* UGeometryGenerator::MakeSphere(AkU32* pMeshDataNum, const AkF32 uRadius, const AkU32 uNumSlices, const AkU32 uNumStacks, const Vector2* pTexScale)
{
	Vector2 vTexScale = Vector2(1.0f);

	if (!pTexScale)
	{
		pTexScale = &vTexScale;
	}

	const AkF32 fDeltaTheta = -DirectX::XM_2PI / float(uNumSlices);
	const AkF32 fDeltaPhi = -DirectX::XM_PI / float(uNumStacks);

	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 1;
	pMeshData = new MeshData_t[uMeshDataNum];

	pMeshData->uVerticeNum = (uNumSlices + 1) * (uNumStacks + 1);
	pMeshData->uIndicesNum = uNumSlices * uNumStacks * 6;
	pMeshData->pVertices = new Vertex_t[pMeshData->uVerticeNum];
	pMeshData->pIndices = new AkU32[pMeshData->uIndicesNum];

	for (AkU32 i = 0; i <= uNumStacks; i++)
	{
		Vector3 vStartPoint = Vector3::Transform(Vector3(0.0f, -uRadius, 0.0f), Matrix::CreateRotationZ(fDeltaPhi * i));
		for (AkU32 j = 0; j <= uNumSlices; j++)
		{
			Vertex_t v = {};
			v.vPosition = Vector3::Transform(vStartPoint, Matrix::CreateRotationY(fDeltaTheta * float(j)));
			v.vNormalModel = v.vPosition;
			v.vTexCoord = Vector2(float(i) / uNumSlices, 1.0f - float(j) / uNumStacks) * *pTexScale;

			pMeshData->pVertices[j + (uNumSlices + 1) * i] = v;
		}
	}

	for (AkU32 i = 0; i < uNumStacks; i++)
	{
		const AkU32 uOffset = (uNumSlices + 1) * i;
		for (AkU32 j = 0; j < uNumSlices; j++)
		{
			pMeshData->pIndices[6 * j + 0 + 6 * uNumSlices * i] = uOffset + j;
			pMeshData->pIndices[6 * j + 1 + 6 * uNumSlices * i] = uOffset + j + uNumSlices + 1;
			pMeshData->pIndices[6 * j + 2 + 6 * uNumSlices * i] = uOffset + j + 1 + uNumSlices + 1;

			pMeshData->pIndices[6 * j + 3 + 6 * uNumSlices * i] = uOffset + j;
			pMeshData->pIndices[6 * j + 4 + 6 * uNumSlices * i] = uOffset + j + 1 + uNumSlices + 1;
			pMeshData->pIndices[6 * j + 5 + 6 * uNumSlices * i] = uOffset + j + 1;
		}
	}

	*pMeshDataNum = uMeshDataNum;

	return pMeshData;
}

MeshData_t* UGeometryGenerator::MakeCube(AkU32* pMeshDataNum, const AkF32 fScale)
{
	MeshData_t* pMeshData = nullptr;
	AkU32 numMeshData = 6;
	pMeshData = new MeshData_t[numMeshData];

	for (AkU32 i = 0; i < numMeshData; i++)
	{
		pMeshData[i].uVerticeNum = 4;
		pMeshData[i].uIndicesNum = 6;
		pMeshData[i].pVertices = new Vertex_t[pMeshData[i].uVerticeNum];
		pMeshData[i].pIndices = new AkU32[pMeshData[i].uIndicesNum];
	}

	// À­¸é
	pMeshData[0].pVertices[0].vPosition = Vector3(-1.0f, 1.0f, -1.0f) * fScale;
	pMeshData[0].pVertices[1].vPosition = Vector3(-1.0f, 1.0f, 1.0f) * fScale;
	pMeshData[0].pVertices[2].vPosition = Vector3(1.0f, 1.0f, 1.0f) * fScale;
	pMeshData[0].pVertices[3].vPosition = Vector3(1.0f, 1.0f, -1.0f) * fScale;
	pMeshData[0].pVertices[0].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pMeshData[0].pVertices[1].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pMeshData[0].pVertices[2].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pMeshData[0].pVertices[3].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pMeshData[0].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[0].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[0].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[0].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// ¾Æ·§¸é
	pMeshData[1].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, -1.0f) * fScale;
	pMeshData[1].pVertices[1].vPosition = Vector3(1.0f, -1.0f, -1.0f) * fScale;
	pMeshData[1].pVertices[2].vPosition = Vector3(1.0f, -1.0f, 1.0f) * fScale;
	pMeshData[1].pVertices[3].vPosition = Vector3(-1.0f, -1.0f, 1.0f) * fScale;
	pMeshData[1].pVertices[0].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData[1].pVertices[1].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData[1].pVertices[2].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData[1].pVertices[3].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData[1].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[1].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[1].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[1].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// ¾Õ¸é
	pMeshData[2].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, -1.0f) * fScale;
	pMeshData[2].pVertices[1].vPosition = Vector3(-1.0f, 1.0f, -1.0f) * fScale;
	pMeshData[2].pVertices[2].vPosition = Vector3(1.0f, 1.0f, -1.0f) * fScale;
	pMeshData[2].pVertices[3].vPosition = Vector3(1.0f, -1.0f, -1.0f) * fScale;
	pMeshData[2].pVertices[0].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[2].pVertices[1].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[2].pVertices[2].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[2].pVertices[3].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[2].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[2].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[2].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[2].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// µÞ¸é
	pMeshData[3].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, 1.0f) * fScale;
	pMeshData[3].pVertices[1].vPosition = Vector3(1.0f, -1.0f, 1.0f) * fScale;
	pMeshData[3].pVertices[2].vPosition = Vector3(1.0f, 1.0f, 1.0f) * fScale;
	pMeshData[3].pVertices[3].vPosition = Vector3(-1.0f, 1.0f, 1.0f) * fScale;
	pMeshData[3].pVertices[0].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[3].pVertices[1].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[3].pVertices[2].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[3].pVertices[3].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[3].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[3].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[3].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[3].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// ¿ÞÂÊ
	pMeshData[4].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, 1.0f) * fScale;
	pMeshData[4].pVertices[1].vPosition = Vector3(-1.0f, 1.0f, 1.0f) * fScale;
	pMeshData[4].pVertices[2].vPosition = Vector3(-1.0f, 1.0f, -1.0f) * fScale;
	pMeshData[4].pVertices[3].vPosition = Vector3(-1.0f, -1.0f, -1.0f) * fScale;
	pMeshData[4].pVertices[0].vNormalModel = Vector3(-1.0f, 0.0f, 0.0f);
	pMeshData[4].pVertices[1].vNormalModel = Vector3(-1.0f, 0.0f, 0.0f);
	pMeshData[4].pVertices[2].vNormalModel = Vector3(-1.0f, 0.0f, 0.0f);
	pMeshData[4].pVertices[3].vNormalModel = Vector3(-1.0f, 0.0f, 0.0f);
	pMeshData[4].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[4].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[4].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[4].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// ¿À¸¥ÂÊ
	pMeshData[5].pVertices[0].vPosition = Vector3(1.0f, -1.0f, 1.0f) * fScale;
	pMeshData[5].pVertices[1].vPosition = Vector3(1.0f, -1.0f, -1.0f) * fScale;
	pMeshData[5].pVertices[2].vPosition = Vector3(1.0f, 1.0f, -1.0f) * fScale;
	pMeshData[5].pVertices[3].vPosition = Vector3(1.0f, 1.0f, 1.0f) * fScale;
	pMeshData[5].pVertices[0].vNormalModel = Vector3(1.0f, 0.0f, 0.0f);
	pMeshData[5].pVertices[1].vNormalModel = Vector3(1.0f, 0.0f, 0.0f);
	pMeshData[5].pVertices[2].vNormalModel = Vector3(1.0f, 0.0f, 0.0f);
	pMeshData[5].pVertices[3].vNormalModel = Vector3(1.0f, 0.0f, 0.0f);
	pMeshData[5].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[5].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[5].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[5].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);

	AkU32 uIndex = 0;
	pMeshData[0].pIndices[uIndex++] = 0; pMeshData[0].pIndices[uIndex++] = 1; pMeshData[0].pIndices[uIndex++] = 2; pMeshData[0].pIndices[uIndex++] = 0; pMeshData[0].pIndices[uIndex++] = 2; pMeshData[0].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[1].pIndices[uIndex++] = 0; pMeshData[1].pIndices[uIndex++] = 1; pMeshData[1].pIndices[uIndex++] = 2; pMeshData[1].pIndices[uIndex++] = 0; pMeshData[1].pIndices[uIndex++] = 2; pMeshData[1].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[2].pIndices[uIndex++] = 0; pMeshData[2].pIndices[uIndex++] = 1; pMeshData[2].pIndices[uIndex++] = 2; pMeshData[2].pIndices[uIndex++] = 0; pMeshData[2].pIndices[uIndex++] = 2; pMeshData[2].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[3].pIndices[uIndex++] = 0; pMeshData[3].pIndices[uIndex++] = 1; pMeshData[3].pIndices[uIndex++] = 2; pMeshData[3].pIndices[uIndex++] = 0; pMeshData[3].pIndices[uIndex++] = 2; pMeshData[3].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[4].pIndices[uIndex++] = 0; pMeshData[4].pIndices[uIndex++] = 1; pMeshData[4].pIndices[uIndex++] = 2; pMeshData[4].pIndices[uIndex++] = 0; pMeshData[4].pIndices[uIndex++] = 2; pMeshData[4].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[5].pIndices[uIndex++] = 0; pMeshData[5].pIndices[uIndex++] = 1; pMeshData[5].pIndices[uIndex++] = 2; pMeshData[5].pIndices[uIndex++] = 0; pMeshData[5].pIndices[uIndex++] = 2; pMeshData[5].pIndices[uIndex++] = 3;

	*pMeshDataNum = numMeshData;

	return pMeshData;
}

MeshData_t* UGeometryGenerator::MakeCubeWidthExtent(AkU32* pMeshDataNum, const Vector3* pExtent)
{
	MeshData_t* pMeshData = nullptr;
	AkU32 numMeshData = 6;
	pMeshData = new MeshData_t[numMeshData];

	for (AkU32 i = 0; i < numMeshData; i++)
	{
		pMeshData[i].uVerticeNum = 4;
		pMeshData[i].uIndicesNum = 6;
		pMeshData[i].pVertices = new Vertex_t[pMeshData[i].uVerticeNum];
		pMeshData[i].pIndices = new AkU32[pMeshData[i].uIndicesNum];
	}

	// À­¸é
	pMeshData[0].pVertices[0].vPosition = Vector3(-1.0f, 1.0f, -1.0f) * (*pExtent);
	pMeshData[0].pVertices[1].vPosition = Vector3(-1.0f, 1.0f, 1.0f) * (*pExtent);
	pMeshData[0].pVertices[2].vPosition = Vector3(1.0f, 1.0f, 1.0f) * (*pExtent);
	pMeshData[0].pVertices[3].vPosition = Vector3(1.0f, 1.0f, -1.0f) * (*pExtent);
	pMeshData[0].pVertices[0].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pMeshData[0].pVertices[1].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pMeshData[0].pVertices[2].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pMeshData[0].pVertices[3].vNormalModel = Vector3(0.0f, 1.0f, 0.0f);
	pMeshData[0].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[0].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[0].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[0].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// ¾Æ·§¸é
	pMeshData[1].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, -1.0f) * (*pExtent);
	pMeshData[1].pVertices[1].vPosition = Vector3(1.0f, -1.0f, -1.0f) * (*pExtent);
	pMeshData[1].pVertices[2].vPosition = Vector3(1.0f, -1.0f, 1.0f) * (*pExtent);
	pMeshData[1].pVertices[3].vPosition = Vector3(-1.0f, -1.0f, 1.0f) * (*pExtent);
	pMeshData[1].pVertices[0].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData[1].pVertices[1].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData[1].pVertices[2].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData[1].pVertices[3].vNormalModel = Vector3(0.0f, -1.0f, 0.0f);
	pMeshData[1].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[1].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[1].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[1].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// ¾Õ¸é
	pMeshData[2].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, -1.0f) * (*pExtent);
	pMeshData[2].pVertices[1].vPosition = Vector3(-1.0f, 1.0f, -1.0f) * (*pExtent);
	pMeshData[2].pVertices[2].vPosition = Vector3(1.0f, 1.0f, -1.0f) * (*pExtent);
	pMeshData[2].pVertices[3].vPosition = Vector3(1.0f, -1.0f, -1.0f) * (*pExtent);
	pMeshData[2].pVertices[0].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[2].pVertices[1].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[2].pVertices[2].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[2].pVertices[3].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[2].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[2].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[2].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[2].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// µÞ¸é
	pMeshData[3].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, 1.0f) * (*pExtent);
	pMeshData[3].pVertices[1].vPosition = Vector3(1.0f, -1.0f, 1.0f) * (*pExtent);
	pMeshData[3].pVertices[2].vPosition = Vector3(1.0f, 1.0f, 1.0f) * (*pExtent);
	pMeshData[3].pVertices[3].vPosition = Vector3(-1.0f, 1.0f, 1.0f) * (*pExtent);
	pMeshData[3].pVertices[0].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[3].pVertices[1].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[3].pVertices[2].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[3].pVertices[3].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[3].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[3].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[3].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[3].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// ¿ÞÂÊ
	pMeshData[4].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, 1.0f) * (*pExtent);
	pMeshData[4].pVertices[1].vPosition = Vector3(-1.0f, 1.0f, 1.0f) * (*pExtent);
	pMeshData[4].pVertices[2].vPosition = Vector3(-1.0f, 1.0f, -1.0f) * (*pExtent);
	pMeshData[4].pVertices[3].vPosition = Vector3(-1.0f, -1.0f, -1.0f) * (*pExtent);
	pMeshData[4].pVertices[0].vNormalModel = Vector3(-1.0f, 0.0f, 0.0f);
	pMeshData[4].pVertices[1].vNormalModel = Vector3(-1.0f, 0.0f, 0.0f);
	pMeshData[4].pVertices[2].vNormalModel = Vector3(-1.0f, 0.0f, 0.0f);
	pMeshData[4].pVertices[3].vNormalModel = Vector3(-1.0f, 0.0f, 0.0f);
	pMeshData[4].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[4].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[4].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[4].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);
	// ¿À¸¥ÂÊ
	pMeshData[5].pVertices[0].vPosition = Vector3(1.0f, -1.0f, 1.0f) * (*pExtent);
	pMeshData[5].pVertices[1].vPosition = Vector3(1.0f, -1.0f, -1.0f) * (*pExtent);
	pMeshData[5].pVertices[2].vPosition = Vector3(1.0f, 1.0f, -1.0f) * (*pExtent);
	pMeshData[5].pVertices[3].vPosition = Vector3(1.0f, 1.0f, 1.0f) * (*pExtent);
	pMeshData[5].pVertices[0].vNormalModel = Vector3(1.0f, 0.0f, 0.0f);
	pMeshData[5].pVertices[1].vNormalModel = Vector3(1.0f, 0.0f, 0.0f);
	pMeshData[5].pVertices[2].vNormalModel = Vector3(1.0f, 0.0f, 0.0f);
	pMeshData[5].pVertices[3].vNormalModel = Vector3(1.0f, 0.0f, 0.0f);
	pMeshData[5].pVertices[0].vTexCoord = Vector2(0.0f, 1.0f);
	pMeshData[5].pVertices[1].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[5].pVertices[2].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[5].pVertices[3].vTexCoord = Vector2(1.0f, 1.0f);

	AkU32 uIndex = 0;
	pMeshData[0].pIndices[uIndex++] = 0; pMeshData[0].pIndices[uIndex++] = 1; pMeshData[0].pIndices[uIndex++] = 2; pMeshData[0].pIndices[uIndex++] = 0; pMeshData[0].pIndices[uIndex++] = 2; pMeshData[0].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[1].pIndices[uIndex++] = 0; pMeshData[1].pIndices[uIndex++] = 1; pMeshData[1].pIndices[uIndex++] = 2; pMeshData[1].pIndices[uIndex++] = 0; pMeshData[1].pIndices[uIndex++] = 2; pMeshData[1].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[2].pIndices[uIndex++] = 0; pMeshData[2].pIndices[uIndex++] = 1; pMeshData[2].pIndices[uIndex++] = 2; pMeshData[2].pIndices[uIndex++] = 0; pMeshData[2].pIndices[uIndex++] = 2; pMeshData[2].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[3].pIndices[uIndex++] = 0; pMeshData[3].pIndices[uIndex++] = 1; pMeshData[3].pIndices[uIndex++] = 2; pMeshData[3].pIndices[uIndex++] = 0; pMeshData[3].pIndices[uIndex++] = 2; pMeshData[3].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[4].pIndices[uIndex++] = 0; pMeshData[4].pIndices[uIndex++] = 1; pMeshData[4].pIndices[uIndex++] = 2; pMeshData[4].pIndices[uIndex++] = 0; pMeshData[4].pIndices[uIndex++] = 2; pMeshData[4].pIndices[uIndex++] = 3;
	uIndex = 0;
	pMeshData[5].pIndices[uIndex++] = 0; pMeshData[5].pIndices[uIndex++] = 1; pMeshData[5].pIndices[uIndex++] = 2; pMeshData[5].pIndices[uIndex++] = 0; pMeshData[5].pIndices[uIndex++] = 2; pMeshData[5].pIndices[uIndex++] = 3;

	*pMeshDataNum = numMeshData;

	return pMeshData;
}

MeshData_t* UGeometryGenerator::MakeWireCube(AkU32* pMeshDataNum, const AkF32 fScale)
{
	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 1;
	pMeshData = new MeshData_t[uMeshDataNum];

	pMeshData->uVerticeNum = 8;
	pMeshData->uIndicesNum = 24;
	pMeshData->pVertices = new Vertex_t[pMeshData->uVerticeNum];
	pMeshData->pIndices = new AkU32[pMeshData->uIndicesNum];

	// ¾Õ¸é
	pMeshData[0].pVertices[0].vPosition = Vector3(-1.0f, -1.0f, -1.0f) * fScale;
	pMeshData[0].pVertices[1].vPosition = Vector3(-1.0f, 1.0f, -1.0f) * fScale;
	pMeshData[0].pVertices[2].vPosition = Vector3(1.0f, 1.0f, -1.0f) * fScale;
	pMeshData[0].pVertices[3].vPosition = Vector3(1.0f, -1.0f, -1.0f) * fScale;
	pMeshData[0].pVertices[0].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[0].pVertices[1].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[0].pVertices[2].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[0].pVertices[3].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[0].pVertices[0].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[0].pVertices[1].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[0].pVertices[2].vTexCoord = Vector2(1.0f, 1.0f);
	pMeshData[0].pVertices[3].vTexCoord = Vector2(0.0f, 1.0f);
	// µÞ¸é
	pMeshData[0].pVertices[4].vPosition = Vector3(-1.0f, -1.0f, 1.0f) * fScale;
	pMeshData[0].pVertices[5].vPosition = Vector3(-1.0f, 1.0f, 1.0f) * fScale;
	pMeshData[0].pVertices[6].vPosition = Vector3(1.0f, 1.0f, 1.0f) * fScale;
	pMeshData[0].pVertices[7].vPosition = Vector3(1.0f, -1.0f, 1.0f) * fScale;
	pMeshData[0].pVertices[4].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[0].pVertices[5].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[0].pVertices[6].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[0].pVertices[7].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[0].pVertices[4].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[0].pVertices[5].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[0].pVertices[6].vTexCoord = Vector2(1.0f, 1.0f);
	pMeshData[0].pVertices[7].vTexCoord = Vector2(0.0f, 1.0f);

	// ¾Õ¸é
	pMeshData[0].pIndices[0] = 0;
	pMeshData[0].pIndices[1] = 1;
	pMeshData[0].pIndices[2] = 1;
	pMeshData[0].pIndices[3] = 2;
	pMeshData[0].pIndices[4] = 2;
	pMeshData[0].pIndices[5] = 3;
	pMeshData[0].pIndices[6] = 3;
	pMeshData[0].pIndices[7] = 0;
	// µÞ¸é
	pMeshData[0].pIndices[8] = 4;
	pMeshData[0].pIndices[9] = 5;
	pMeshData[0].pIndices[10] = 5;
	pMeshData[0].pIndices[11] = 6;
	pMeshData[0].pIndices[12] = 6;
	pMeshData[0].pIndices[13] = 7;
	pMeshData[0].pIndices[14] = 7;
	pMeshData[0].pIndices[15] = 4;
	// ¿·¸é
	pMeshData[0].pIndices[16] = 0;
	pMeshData[0].pIndices[17] = 4;
	pMeshData[0].pIndices[18] = 1;
	pMeshData[0].pIndices[19] = 5;
	pMeshData[0].pIndices[20] = 2;
	pMeshData[0].pIndices[21] = 6;
	pMeshData[0].pIndices[22] = 3;
	pMeshData[0].pIndices[23] = 7;

	*pMeshDataNum = uMeshDataNum;

	return pMeshData;
}

MeshData_t* UGeometryGenerator::MakeWireCubeWidthExtent(AkU32* pMeshDataNum, Vector3* vP0, Vector3* vP1, Vector3* vP2, Vector3* vP3, Vector3* vP4, Vector3* vP5, Vector3* vP6, Vector3* vP7)
{
	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 1;
	pMeshData = new MeshData_t[uMeshDataNum];

	pMeshData->uVerticeNum = 8;
	pMeshData->uIndicesNum = 24;
	pMeshData->pVertices = new Vertex_t[pMeshData->uVerticeNum];
	pMeshData->pIndices = new AkU32[pMeshData->uIndicesNum];

	// ¾Õ¸é
	pMeshData[0].pVertices[0].vPosition = *vP0;
	pMeshData[0].pVertices[1].vPosition = *vP1;
	pMeshData[0].pVertices[2].vPosition = *vP2;
	pMeshData[0].pVertices[3].vPosition = *vP3;
	pMeshData[0].pVertices[0].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[0].pVertices[1].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[0].pVertices[2].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[0].pVertices[3].vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
	pMeshData[0].pVertices[0].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[0].pVertices[1].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[0].pVertices[2].vTexCoord = Vector2(1.0f, 1.0f);
	pMeshData[0].pVertices[3].vTexCoord = Vector2(0.0f, 1.0f);
	// µÞ¸é
	pMeshData[0].pVertices[4].vPosition = *vP4;
	pMeshData[0].pVertices[5].vPosition = *vP5;
	pMeshData[0].pVertices[6].vPosition = *vP6;
	pMeshData[0].pVertices[7].vPosition = *vP7;
	pMeshData[0].pVertices[4].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[0].pVertices[5].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[0].pVertices[6].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[0].pVertices[7].vNormalModel = Vector3(0.0f, 0.0f, 1.0f);
	pMeshData[0].pVertices[4].vTexCoord = Vector2(0.0f, 0.0f);
	pMeshData[0].pVertices[5].vTexCoord = Vector2(1.0f, 0.0f);
	pMeshData[0].pVertices[6].vTexCoord = Vector2(1.0f, 1.0f);
	pMeshData[0].pVertices[7].vTexCoord = Vector2(0.0f, 1.0f);

	// ¾Õ¸é
	pMeshData[0].pIndices[0] = 0;
	pMeshData[0].pIndices[1] = 1;
	pMeshData[0].pIndices[2] = 1;
	pMeshData[0].pIndices[3] = 2;
	pMeshData[0].pIndices[4] = 2;
	pMeshData[0].pIndices[5] = 3;
	pMeshData[0].pIndices[6] = 3;
	pMeshData[0].pIndices[7] = 0;
	// µÞ¸é
	pMeshData[0].pIndices[8] = 4;
	pMeshData[0].pIndices[9] = 5;
	pMeshData[0].pIndices[10] = 5;
	pMeshData[0].pIndices[11] = 6;
	pMeshData[0].pIndices[12] = 6;
	pMeshData[0].pIndices[13] = 7;
	pMeshData[0].pIndices[14] = 7;
	pMeshData[0].pIndices[15] = 4;
	// ¿·¸é
	pMeshData[0].pIndices[16] = 0;
	pMeshData[0].pIndices[17] = 4;
	pMeshData[0].pIndices[18] = 1;
	pMeshData[0].pIndices[19] = 5;
	pMeshData[0].pIndices[20] = 2;
	pMeshData[0].pIndices[21] = 6;
	pMeshData[0].pIndices[22] = 3;
	pMeshData[0].pIndices[23] = 7;

	*pMeshDataNum = uMeshDataNum;

	return pMeshData;
}

MeshData_t* UGeometryGenerator::MakeWireSphere(AkU32* pMeshDataNum, const AkF32 fScale)
{
	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 1;
	pMeshData = new MeshData_t[uMeshDataNum];

	const AkU32 uPointsNum = 30;
	const AkF32 fDeltaTheta = -DirectX::XM_2PI / AkF32(uPointsNum);

	pMeshData->uVerticeNum = 3 * uPointsNum;
	pMeshData->uIndicesNum = 2 * pMeshData->uVerticeNum;
	pMeshData->pVertices = new Vertex_t[pMeshData->uVerticeNum];
	pMeshData->pIndices = new AkU32[pMeshData->uIndicesNum];

	// XY Plane
	Vector3 vStartPoint = Vector3(1.0f, 0.0f, 0.0f) * fScale;
	for (AkU32 i = 0; i < uPointsNum; i++)
	{
		Vertex_t v = {};
		v.vPosition = Vector3::Transform(vStartPoint, Matrix::CreateRotationZ(fDeltaTheta * float(i)));
		v.vNormalModel = v.vPosition;

		pMeshData->pVertices[i] = v;
		pMeshData->pIndices[2 * i] = i;
		pMeshData->pIndices[2 * i + 1] = ((uPointsNum - 1) == i) ? 0 : (i + 1);
	}

	// YZ Plane
	vStartPoint = Vector3(0.0f, 1.0f, 0.0f) * fScale;
	for (AkU32 i = 0; i < uPointsNum; i++)
	{
		Vertex_t v = {};
		v.vPosition = Vector3::Transform(vStartPoint, Matrix::CreateRotationX(fDeltaTheta * float(i)));
		v.vNormalModel = v.vPosition;

		pMeshData->pVertices[uPointsNum + i] = v;
		pMeshData->pIndices[2 * uPointsNum + 2 * i] = uPointsNum + i;
		pMeshData->pIndices[2 * uPointsNum + 2 * i + 1] = ((uPointsNum - 1) == i) ? uPointsNum : (uPointsNum + i + 1);
	}

	// XZ Plane
	vStartPoint = Vector3(1.0f, 0.0f, 0.0f) * fScale;
	for (AkU32 i = 0; i < uPointsNum; i++)
	{
		Vertex_t v = {};
		v.vPosition = Vector3::Transform(vStartPoint, Matrix::CreateRotationY(fDeltaTheta * float(i)));
		v.vNormalModel = v.vPosition;

		pMeshData->pVertices[2 * uPointsNum + i] = v;
		pMeshData->pIndices[4 * uPointsNum + 2 * i] = 2 * uPointsNum + i;
		pMeshData->pIndices[4 * uPointsNum + 2 * i + 1] = ((uPointsNum - 1) == i) ? 2 * uPointsNum : (2 * uPointsNum + i + 1);
	}

	*pMeshDataNum = uMeshDataNum;

	return pMeshData;
}

MeshData_t* UGeometryGenerator::MakeGrid(AkU32* pMeshDataNum, const AkF32 fScale, const AkU32 uNumSlices, const AkU32 uNumStacks, const Vector2* pTexScale)
{
	Vector2 vTexScale = Vector2(1.0f);

	if (!pTexScale)
	{
		pTexScale = &vTexScale;
	}

	MeshData_t* pMeshData = nullptr;
	AkU32 numMeshData = 1;
	pMeshData = new MeshData_t[numMeshData];

	pMeshData->uVerticeNum = (uNumSlices + 1) * (uNumStacks + 1);
	pMeshData->uIndicesNum = uNumSlices * uNumStacks * 6;
	pMeshData->pVertices = new Vertex_t[pMeshData->uVerticeNum];
	pMeshData->pIndices = new AkU32[pMeshData->uIndicesNum];

	AkF32 dx = 2.0f / uNumSlices;
	AkF32 dy = 2.0f / uNumStacks;

	float y = -1.0f;
	for (AkU32 i = 0; i <= uNumStacks; i++)
	{
		float x = -1.0f;
		for (AkU32 j = 0; j <= uNumSlices; j++)
		{
			Vertex_t v;
			v.vPosition = Vector3(x, y, 0.0f) * fScale * 0.5f;
			v.vNormalModel = Vector3(0.0f, 0.0f, -1.0f);
			v.vTangentModel = Vector3(1.0f, 0.0f, 0.0f);
			v.vTexCoord = Vector2(x + 1.0f, y + 1.0f) * 0.5f * *pTexScale;

			pMeshData->pVertices[j + (uNumSlices + 1) * i] = v;

			x += dx;
		}
		y += dy;
	}

	for (AkU32 i = 0; i < uNumStacks; i++)
	{
		const AkU32 uOffset = (uNumSlices + 1) * i;
		for (AkU32 j = 0; j < uNumSlices; j++)
		{
			pMeshData->pIndices[6 * j + 0 + 6 * uNumSlices * i] = uOffset + j;
			pMeshData->pIndices[6 * j + 1 + 6 * uNumSlices * i] = uOffset + j + uNumSlices + 1;
			pMeshData->pIndices[6 * j + 2 + 6 * uNumSlices * i] = uOffset + j + 1 + uNumSlices + 1;

			pMeshData->pIndices[6 * j + 3 + 6 * uNumSlices * i] = uOffset + j;
			pMeshData->pIndices[6 * j + 4 + 6 * uNumSlices * i] = uOffset + j + 1 + uNumSlices + 1;
			pMeshData->pIndices[6 * j + 5 + 6 * uNumSlices * i] = uOffset + j + 1;
		}
	}

	*pMeshDataNum = numMeshData;

	return pMeshData;
}

// ºñµ¿±â Ã³¸® ÇÊ¿ä!!
MeshData_t* UGeometryGenerator::ReadFromFile(UApplication* pApp, AkU32* pMeshDataNum, const wchar_t* wcBasePath, const wchar_t* wcFilename, AkBool bIsAnim, Matrix* pDefaultMat, Matrix const** pBoneOffsetMat, AkI32 const** pBoneHierarchy, AkU32* pBoneNum)
{
	UModelImporter tModelImporter;
	MeshData_t* pMeshData = nullptr;

	tModelImporter.Load(pApp, wcBasePath, wcFilename, bIsAnim);

	pMeshData = tModelImporter.GetMeshData();

	*pMeshDataNum = tModelImporter.GetMeshDataNum();

	NormalizeMeshData(pMeshData, *pMeshDataNum, 1.0f, bIsAnim, pDefaultMat);

	if (bIsAnim)
	{
		*pBoneOffsetMat = tModelImporter.GetBoneOffsetTransformList();
		*pBoneHierarchy = tModelImporter.GetBoneHierarchyList();
		*pBoneNum = tModelImporter.GetBoneNum();
	}

	return pMeshData;
}

void UGeometryGenerator::DestroyGeometry(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	if (pMeshData)
	{
		for (AkU32 i = 0; i < uMeshDataNum; i++)
		{
			Vertex_t* pVertices = pMeshData[i].pVertices;
			SkinnedVertex_t* pSkinnedVertices = pMeshData[i].pSkinnedVertices;
			AkU32* pIndices = pMeshData[i].pIndices;
			if (pVertices)
			{
				delete[] pVertices;
			}
			if (pSkinnedVertices)
			{
				delete[] pSkinnedVertices;
			}
			if (pIndices)
			{
				delete[] pIndices;
			}
			/*if (pMeshData[i].wcAlbedoTextureFilename)
			{
				free(pMeshData[i].wcAlbedoTextureFilename);
				pMeshData[i].wcAlbedoTextureFilename = nullptr;
			}
			if (pMeshData[i].wcEmissiveTextureFilename)
			{
				free(pMeshData[i].wcEmissiveTextureFilename);
				pMeshData[i].wcEmissiveTextureFilename = nullptr;
			}
			if (pMeshData[i].wcHeightTextureFilename)
			{
				free(pMeshData[i].wcHeightTextureFilename);
				pMeshData[i].wcHeightTextureFilename = nullptr;
			}
			if (pMeshData[i].wcNormalTextureFilename)
			{
				free(pMeshData[i].wcNormalTextureFilename);
				pMeshData[i].wcNormalTextureFilename = nullptr;
			}
			if (pMeshData[i].wcMetallicTextureFilename)
			{
				free(pMeshData[i].wcMetallicTextureFilename);
				pMeshData[i].wcMetallicTextureFilename = nullptr;
			}
			if (pMeshData[i].wcRoughnessTextureFilename)
			{
				free(pMeshData[i].wcRoughnessTextureFilename);
				pMeshData[i].wcRoughnessTextureFilename = nullptr;
			}
			if (pMeshData[i].wcAoTextureFilename)
			{
				free(pMeshData[i].wcAoTextureFilename);
				pMeshData[i].wcAoTextureFilename = nullptr;
			}
			if (pMeshData[i].wcOpacityTextureFilename)
			{
				free(pMeshData[i].wcOpacityTextureFilename);
				pMeshData[i].wcOpacityTextureFilename = nullptr;
			}*/
		}
		delete[] pMeshData;
	}
}
