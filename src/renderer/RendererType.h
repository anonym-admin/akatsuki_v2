#pragma once

const AkU32 SWAP_CHAIN_FRAME_COUNT = 3;
const AkU32 PENDING_FRAME_COUNT = SWAP_CHAIN_FRAME_COUNT - 1;
const AkU32 POINT_LIGHTS_NUM = 1;
const AkU32 SPOT_LIGHTS_NUM = 1;
const AkU32 MAX_LIGHTS_COUNT = 3;

enum class CONSTANT_BUFFER_TYPE
{
	CONSTANT_BUFFER_TYPE_GLOBAL,
	CONSTANT_BUFFER_TYPE_MESH,
	CONSTANT_BUFFER_TYPE_SKINNED_MESH,
	CONSTANT_BUFFER_TYPE_MATERIAL,
	CONSTANT_BUFFER_TYPE_SPRITE,
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
	AkF32 fDummy0 = 0.0f;
	AkF32 fDummy1 = 0.0f;
	Matrix mViewProj[5] = {};
};

struct GlobalConstantBuffer_t
{
	Matrix mView = Matrix();
	Matrix mProj = Matrix();
	Vector3 vEyeWorld = Vector3(0.0f);
	AkF32 fStrengthIBL = 1.0f;
	Vector3 dummy0;
	AkF32 dummy1;
	Vector3 dummy2;
	AkF32 dummy3;
	Vector3 dummy4;
	AkF32 dummy5;

	Light_t tLights[MAX_LIGHTS_COUNT] = {};
};

struct MeshConstantBuffer_t
{
	Matrix mWorld = Matrix();
	Matrix mWorldIT = Matrix();
};

struct MaterialConstantBuffer_t
{
	Vector3 vAlbedoFactor = Vector3(1.0f);
	AkF32 fRoughnessFactor = 0.0f;
	Vector3 vEmissionFactor = Vector3(0.0f);
	AkF32 fMetallicFactor = 0.0f;

	AkU32 uUseAlbedoMap = AK_FALSE;
	AkU32 uUseNormalMap = AK_FALSE;
	AkU32 uUseEimissiveMap = AK_FALSE;
	AkU32 uUseAOMap = AK_FALSE;
	AkU32 uInvertNormalMapY = AK_FALSE;
	AkU32 uUseMetallicMap = AK_FALSE;
	AkU32 uUseRoughnessMap = AK_FALSE;
	AkF32 fReserve0 = 0.0f;
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
};

struct FontHandle_t
{
	IDWriteTextFormat* pTextFormat;
	float fFontSize;
	WCHAR wchFontFamilyName[512];
};
