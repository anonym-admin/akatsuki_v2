#pragma once

const AkU32 SWAP_CHAIN_FRAME_COUNT = 3;
const AkU32 PENDING_FRAME_COUNT = SWAP_CHAIN_FRAME_COUNT - 1;
const AkU32 DIRECTIONAL_LIGHTS_NUM = 1; // Global
const AkU32 POINT_LIGHTS_NUM = 1;
const AkU32 SPOT_LIGHTS_NUM = 1;
const AkU32 MAX_LIGHTS_COUNT = DIRECTIONAL_LIGHTS_NUM + POINT_LIGHTS_NUM + SPOT_LIGHTS_NUM;

enum class CONSTANT_BUFFER_TYPE
{
	CONSTANT_BUFFER_TYPE_GLOBAL,
	CONSTANT_BUFFER_TYPE_MESH,
	CONSTANT_BUFFER_TYPE_SKINNED_MESH,
	CONSTANT_BUFFER_TYPE_LINE,
	CONSTANT_BUFFER_TYPE_MATERIAL,
	CONSTANT_BUFFER_TYPE_SPRITE,
	CONSTANT_BUFFER_TYPE_TERRAIN_BRUSH,
	CONSTANT_BUFFER_TYPE_POSTEFFECT,
	CONSTANT_BUFFER_TYPE_POSTPROCESS,
	CONSTANT_BUFFER_TYPE_PARTICLE_COLOR,
	CONSTANT_BUFFER_TYPE_PARTICLE_SPARK,
	CONSTANT_BUFFER_TYPE_PARTICLE_SPRITE,
	CONSTANT_BUFFER_TYPE_VOLUME_CLOUD,
	CONSTANT_BUFFER_TYPE_COUNT
};

struct ConstantBufferProperty_t
{
	CONSTANT_BUFFER_TYPE eCBType = {};
	AkU32 uCBTypeSize = 0;
};

struct Light_t
{
	Vector3 vRadiance = Vector3(5.0f); // Strength
	AkF32 fFallOffStart = 0.0f;
	Vector3 vDirection = Vector3(0.0f, 0.0f, 1.0f);
	AkF32 fFallOffEnd = 20.0f;
	Vector3 vPosition = Vector3(0.0f, 0.0f, -2.0f);
	AkF32 fSpotPower = 6.0f;
	AkU32 uType = LIGHT_OFF;
	AkF32 fRadius = 0.0f;
	AkF32 fHaloRadius = 0.0f;
	AkF32 fHaloStrength = 0.0f;
	Matrix mViewProj[5] = {};
};

struct GlobalConstantBuffer_t
{
	Matrix mView = Matrix();
	Matrix mProj = Matrix();
	Vector3 vEyeWorld = Vector3(0.0f);
	AkF32 fStrengthIBL = 1.0f;
	Matrix mInvView = Matrix();
	Matrix mInvProj = Matrix();

	AkF32 fTime = 0.0f;
	Vector3 vDummy;

	Light_t tLights[MAX_LIGHTS_COUNT] = {};
};

struct MeshConstantBuffer_t
{
	Matrix mWorld = Matrix();
	Matrix mWorldIT = Matrix();
	Matrix mWorldInv = Matrix();

	AkF32 fHeightScale = 1.0f;
	Vector3 vClipMin = Vector3(0.0f);
	AkF32 fWindTrunk = 0.0f;
	Vector3 vClipMax = Vector3(0.0f);
	AkF32 fWindLeaves = 0.0f;
	Vector3 vDummy = Vector3(0.0f);
};

struct LineConstantBuffer_t
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
};

struct MaterialConstantBuffer_t
{
	Vector3 vAlbedoFactor = Vector3(1.0f);
	AkF32 fRoughnessFactor = 0.0f;
	Vector3 vEmissionFactor = Vector3(0.0f);
	AkF32 fMetallicFactor = 0.0f;

	AkU32 uUseAlbedoMap = AK_FALSE;
	AkU32 uUseNormalMap = AK_FALSE;
	AkU32 uUseEmissiveMap = AK_FALSE;
	AkU32 uUseAOMap = AK_FALSE;

	AkU32 uInvertNormalMapY = AK_FALSE;
	AkU32 uUseMetallicMap = AK_FALSE;
	AkU32 uUseRoughnessMap = AK_FALSE;
	AkU32 uUseHeightMap = AK_FALSE;
};

struct SkinnedMeshConstantBuffer_t
{
	Matrix mBonesTransform[96] = {};
};

struct SpriteConstantBuffer_t
{
	Vector2 vScreenRes;
	Vector2 vPos;
	Vector2 vScale;
	Vector2 vTexSize;
	Vector2 vTexSampePos;
	Vector2 vTexSampleSize;
	AkF32 fZ;
	AkF32 fAlpha;
	AkF32 fReserved0;
	AkF32 fReserved1;
	AkU32 uDrawBackground;
	Vector3 vFontColor;
};

struct BrushConstantBuffer_t // Terrain.h
{
	AkI32 iType = 0;
	Vector3 vPos = Vector3(0.0f);
	AkF32 fRange = 10.0f;
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
};

struct PostEffectConstantBuffer_t
{
	AkI32 iMode;
	AkF32 fDepthScale;
	AkF32 fFogStrength;
};

struct PostProcessConstantBuffer_t
{
	AkF32 fDx;
	AkF32 fDy;
	AkF32 fThreshold;
	AkF32 fStrength;
	AkF32 fExposure;
	AkF32 fGamma;
	AkU32 uOption0;
	AkF32 fOption1;
};

struct ParticleColorConstantBuffer_t
{
	Vector4 vTotalColor = Vector4(1.0f);
	Vector4 vColorOverLifeTime = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
};

struct ParticleSparkConstantBuffer_t
{
	AkF32 fTime = 0.0f;
	AkF32 fDuration = 0.0f;
	Vector2 vStartSize = Vector2(0.0f);
	Vector3 vStartDirection = Vector3(0.0f);
	AkF32 fSizeOverLifeTime = 0.0f;
	Vector3 vRotOverLifeTime = Vector3(0.0f);
	AkF32 fPadding = 0.0f;
};

struct ParticleSpriteConstantBuffer_t
{
	Vector2 vMaxFrame = Vector2(0.0f);
	Vector2 vCurFrame = Vector2(0.0f);
};

struct VolumeCloudConstantBuffer_t
{
	Vector3 vUVWoffset = Vector3(0.0f);
	AkF32 fLightAbsorptionCoeff = 5.0f;
	Vector3 vLightDir = Vector3(0.0f, 1.0f, 0.0f);
	AkF32 fDensityAbsorption = 10.0f;
	Vector3 vLightColor = Vector3(1.0f, 1.0f, 1.0f) * 40.0f;
	AkF32 fAniso = 0.3f;
};

struct TextureHandle_t
{
	ID3D12Resource* pTextureResource = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
	AkBool bUpdated = AK_FALSE;
	AkBool bFromFile = AK_FALSE;
	AkU32 uRefCount = 0;
	void* pSearchHandle = nullptr;
	List_t tLink = {};
};

struct Mesh_t
{
	ID3D12Resource* pVB = nullptr;
	ID3D12Resource* pIB = nullptr;
	D3D12_VERTEX_BUFFER_VIEW tVBView = {};
	D3D12_INDEX_BUFFER_VIEW tIBView = {};
	AkU32 uVertexCountPerInstance = 0;
	AkU32 uIndexCountPerInstance = 0;
	TextureHandle_t* pAldedoTextureHandle = nullptr;
	TextureHandle_t* pNormalTextureHandle = nullptr;
	TextureHandle_t* pEmissiveTextureHandle = nullptr;
	TextureHandle_t* pMetallicTextureHandle = nullptr;
	TextureHandle_t* pRoughnessTextureHandle = nullptr;
	TextureHandle_t* pAoTextureHandle = nullptr;
	TextureHandle_t* pHeightTextureHandle = nullptr;
};

struct FontHandle_t
{
	IDWriteTextFormat* pTextFormat;
	float fFontSize;
	WCHAR wchFontFamilyName[512];
};

struct DynamicVertexBufferHandle_t
{
	ID3D12Resource* pUploadBuffer = nullptr;
	AkU32 uSizePerVertex = 0;
	AkU32 uVertexNum = 0;
	AkBool bUpdated = AK_FALSE;
};

struct DynamicDefaultBufferHandle_t
{
	ID3D12Resource* pUploadBuffer = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
	AkU32 uSizePerType = 0;
	AkU32 uDataNum = 0;
	AkBool bUpdated = AK_FALSE;
};
