#pragma once

#include "AkVertex.h"

struct MeshData_t
{
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

	//wchar_t* wcAlbedoTextureFilename = nullptr;
	//wchar_t* wcEmissiveTextureFilename = nullptr;
	//wchar_t* wcHeightTextureFilename = nullptr;
	//wchar_t* wcNormalTextureFilename = nullptr;
	//wchar_t* wcMetallicTextureFilename = nullptr;
	//wchar_t* wcRoughnessTextureFilename = nullptr;
	//wchar_t* wcAoTextureFilename = nullptr;
	//wchar_t* wcOpacityTextureFilename = nullptr;
};