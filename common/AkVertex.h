#pragma once

#include "AkType.h"
#include <directxtk/SimpleMath.h>

using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;

struct Vertex_t
{
	Vector3 vPosition = Vector3(0.0f);
	Vector3 vNormalModel = Vector3(0.0f);
	Vector2 vTexCoord = Vector2(0.0f);
	Vector3 vTangentModel = Vector3(0.0f);
};

struct SkinnedVertex_t
{
	Vector3 vPosition = Vector3(0.0f);
	Vector3 vNormalModel = Vector3(0.0f);
	Vector2 vTexCoord = Vector2(0.0f);
	Vector3 vTangentModel = Vector3(0.0f);
	AkF32 fBlendWeights[8] = {};
	AkU8 uBoneIndices[8] = {};
};

struct SpriteVertex_t
{
	Vector3 vPosition = Vector3(0.0f);
	Vector4 vColors = Vector4(0.0f);
	Vector2 vTexCoord = Vector2(0.0f);
};

struct PostProcessVertex_t
{
	Vector3 vPosition = Vector3(0.0f);
	Vector2 vTexCoord = Vector2(0.0f);
};

struct LineVertex_t
{
	Vector3 vPosition = Vector3(0.0f);
	Vector3 vColor = Vector3(0.0f);
};
