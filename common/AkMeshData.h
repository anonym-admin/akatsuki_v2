#pragma once

#include "AkVertex.h"

struct MeshData_t
{
	//wchar_t wcMeshName[_MAX_PATH] = {};
	//wchar_t wcMaterialName[_MAX_PATH] = {};

	Vertex_t* pVertices = nullptr;
	SkinnedVertex_t* pSkinnedVertices = nullptr;
	AkU32* pIndices = nullptr;
	AkU32 uVerticeNum = 0;
	AkU32 uIndicesNum = 0;

	wchar_t wcAlbedoTextureFilename[_MAX_PATH] = {};
	wchar_t wcEmissiveTextureFilename[_MAX_PATH] = {};
	wchar_t wcHeightTextureFilename[_MAX_PATH] = {};
	wchar_t wcNormalTextureFilename[_MAX_PATH] = {};
	wchar_t wcMetallicTextureFilename[_MAX_PATH] = {};
	wchar_t wcRoughnessTextureFilename[_MAX_PATH] = {};
	wchar_t wcAoTextureFilename[_MAX_PATH] = {};
	wchar_t wcOpacityTextureFilename[_MAX_PATH] = {};
};

struct VertexWeight_t
{
	AkF32 fBlendWeights[8] = {};
	AkU8 uBoneIndices[8] = {};
	AkU8 uWeightNum = 0;
	AkU8 uIndiceNum = 0;
};

struct BoneData_t
{
	wchar_t wcName[_MAX_PATH] = {};
	AkI32 iIndex = -1;
	Matrix mOffset = Matrix();
};

struct MaterialData_t
{
	wchar_t wcName[_MAX_PATH] = {};
	Vector4 vAmbient = Vector4(0.0f);
	Vector4 vDiffuse = Vector4(0.0f);
	Vector4 vSpecular = Vector4(0.0f);
	Vector4 vEmissive = Vector4(0.0f);

	wchar_t wcAlbedoMapName[_MAX_PATH] = {};
	wchar_t wcEmissiveMapName[_MAX_PATH] = {};
	wchar_t wcHeightMapName[_MAX_PATH] = {};
	wchar_t wcNormalMapName[_MAX_PATH] = {};
	wchar_t wcMetallicMapName[_MAX_PATH] = {};
	wchar_t wcRoughnessMapName[_MAX_PATH] = {};
	wchar_t wcAoMapName[_MAX_PATH] = {};
	wchar_t wcOpacityMapName[_MAX_PATH] = {};
};

struct New_MeshData_t
{
	wchar_t wcMeshName[_MAX_PATH] = {};
	wchar_t wcMaterialName[_MAX_PATH] = {};

	Vertex_t* pVertices = nullptr;
	SkinnedVertex_t* pSkinnedVertices = nullptr;
	AkU32* pIndices = nullptr;
	AkU32 uVerticeNum = 0;
	AkU32 uIndicesNum = 0;

	wchar_t wcAlbedoTextureFilename[_MAX_PATH] = {};
	wchar_t wcEmissiveTextureFilename[_MAX_PATH] = {};
	wchar_t wcHeightTextureFilename[_MAX_PATH] = {};
	wchar_t wcNormalTextureFilename[_MAX_PATH] = {};
	wchar_t wcMetallicTextureFilename[_MAX_PATH] = {};
	wchar_t wcRoughnessTextureFilename[_MAX_PATH] = {};
	wchar_t wcAoTextureFilename[_MAX_PATH] = {};
	wchar_t wcOpacityTextureFilename[_MAX_PATH] = {};
};

struct NodeData_t
{
	AkI32 iIndex = -1;
	wchar_t wcName[_MAX_PATH] = {};
	AkI32 iParent = -1;
	Matrix mTransform = Matrix();
};

struct BillboardData_t
{
	BillboardVertex_t* pVertice = nullptr;
	AkU32 uPointsNum = 0;

	wchar_t wcAlbedoTextureFilename[_MAX_PATH] = {};
	wchar_t wcEmissiveTextureFilename[_MAX_PATH] = {};
	wchar_t wcHeightTextureFilename[_MAX_PATH] = {};
	wchar_t wcNormalTextureFilename[_MAX_PATH] = {};
	wchar_t wcMetallicTextureFilename[_MAX_PATH] = {};
	wchar_t wcRoughnessTextureFilename[_MAX_PATH] = {};
	wchar_t wcAoTextureFilename[_MAX_PATH] = {};
	wchar_t wcOpacityTextureFilename[_MAX_PATH] = {};
};

struct LineData_t
{
	LineVertex_t* pVertices = nullptr;
	AkU32* pIndices = nullptr;
	AkU32 uVerticeNum = 0;
	AkU32 uIndicesNum = 0;
};

