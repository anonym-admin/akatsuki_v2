#pragma once

#include "CommonVertex.h"

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

struct LineData_t
{
	VertexColor_t* pVertices = nullptr;
	AkU32* pIndices = nullptr;
	AkU32 uVerticeNum = 0;
	AkU32 uIndicesNum = 0;
};

struct BillboardData_t
{
	VertexSize_t* pVertice = nullptr;
	AkU32 uPointsNum = 0;

	wchar_t wcAlbedoTextureFilename[_MAX_PATH] = {};
	wchar_t wcEmissiveTextureFilename[_MAX_PATH] = {};
	wchar_t wcHeightTextureFilename[_MAX_PATH] = {};
	wchar_t wcNormalTextureFilename[_MAX_PATH] = {};
	wchar_t wcMetallicTextureFilename[_MAX_PATH] = {};
	wchar_t wcRoughnessTextureFilename[_MAX_PATH] = {};
	wchar_t wcAoTextureFilename[_MAX_PATH] = {};
	wchar_t wcOpacityTextureFilename[_MAX_PATH] = {};
	wchar_t wcArrayFilename[_MAX_PATH] = {};
};
