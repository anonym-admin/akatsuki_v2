#pragma once

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <map>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

#include <directxtk/SimpleMath.h>
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

struct Vertex
{
	Vector3 position;
	Vector3 normalModel;
	Vector2 texcoord;
	Vector3 tangentModel;
};

struct SkinnedVertex
{
	Vector3 position;
	Vector3 normalModel;
	Vector2 texcoord;
	Vector3 tangentModel;
	float blendWeights[8] = { 0.0f, 0.0f, 0.0f, 0.0f,
							 0.0f, 0.0f, 0.0f, 0.0f };  // BLENDWEIGHT0 and 1
	uint8_t boneIndices[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // BLENDINDICES0 and 1
};

struct MeshData
{
	std::vector<Vertex> vertices;
	std::vector<SkinnedVertex> skinnedVertices;
	std::vector<unsigned int> indices;
	std::string albedoTextureFilename = "";
	std::string emissiveTextureFilename = "";
	std::string heightTextureFilename = "";
	std::string normalTextureFilename = "";
	std::string metallicTextureFilename = "";
	std::string roughnessTextureFilename = "";
	std::string aoTextureFilename = "";
	std::string opacityTextureFilename = "";
};