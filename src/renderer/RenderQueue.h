#pragma once

enum class RENDER_ITEM_TYPE
{
	RENDER_ITEM_TYPE_MESH_OBJ,
	RENDER_ITEM_TYPE_SKINNED_MESH_OBJ,
	RENDER_ITEM_TYPE_SKYBOX_OBJ,
	RENDER_ITEM_TYPE_SPRITE_OBJ,
	RENDER_ITEM_TYPE_LINE_OBJ,
	RENDER_ITEM_TYPE_BILLBOARD,
	RENDER_ITEM_TYPE_TERRAIN_OBJ,
	RENDER_ITEM_TYPE_PARTICLE,
};

struct RenderMeshObjParam_t
{
	const Matrix* pWorld = nullptr;
	AkBool bDrawNormal = AK_FALSE;
};

struct RenderSkinnedMeshObjParam_t
{
	const Matrix* pWorld = nullptr;
	const Matrix* pBonesTransform = nullptr;
	AkBool bDrawNormal = AK_FALSE;
};

struct RenderSkyboxObjParam_t
{
	const Matrix* pWorld = nullptr;
	void* pEnvHDR = nullptr;
	void* pDiffuseHDR = nullptr;
	void* pSpecularHDR = nullptr;
};

struct RenderSpriteObjParam_t
{
	AkI32 iPosX;
	AkI32 iPosY;
	AkF32 fScaleX;
	AkF32 fScaleY;
	RECT tRect;
	AkBool bUseRect;
	AkF32 fZ;
	void* pTexHandle;
	AkBool bUseBlend;
};

struct RenderLineObjParam_t
{
	const Matrix* pWorld = nullptr;
};

struct RenderBillboardParam_t
{
	const Matrix* pWorld = nullptr;
};

struct RenderTerrainParam_t
{
	const Matrix* pWorld = nullptr;
	void* pBrush = nullptr;
	AkBool bDrawNormal = AK_FALSE;
};

struct ParticleParam_t
{
	const Matrix* pWorldRow = nullptr;
	DynamicDefaultBufferHandle_t* pDBHandle = nullptr;
	AkU32 uParticleNum = 0;
	AkF32 fTime = 0.0f;
	AkF32 fDuration = 0.0f;
	const Vector2* pStartSize = nullptr;
	const Vector3* pStartDirection = nullptr;
	AkF32 fSizeOverLifeTime = 0.0f;
	const Vector3* pRotOverLifeTime = nullptr;
	const Vector4* pTotalColor = nullptr;
	const Vector4* pColorOverLifeTime = nullptr;
	const Vector2* pMaxFrame = nullptr;
	const Vector2* pCurFrame = nullptr;
};

struct RenderItem_t
{
	RENDER_ITEM_TYPE eItemType = {};
	void* pObjHandle = nullptr;
	union
	{
		RenderMeshObjParam_t tMeshObjParam;
		RenderSkinnedMeshObjParam_t tSkinnedMeshObjParam;
		RenderSkyboxObjParam_t tSkyboxObjParam;
		RenderSpriteObjParam_t tSpriteObjParam;
		RenderLineObjParam_t tLineObjParam;
		RenderBillboardParam_t tBillboardParam;
		RenderTerrainParam_t tTerrianParam;
		ParticleParam_t tParticleParam;
	};
};

/*
==============
RenderQueue
==============
*/

class FRenderer;
class FCommandListPool;

class FRenderQueue
{
public:
	FRenderQueue();
	~FRenderQueue();

	AkBool Initialize(FRenderer* pRenderer, DWORD dwMaxItemNum);
	AkBool Add(const RenderItem_t* pItem);
	DWORD Process(DWORD uThreadIndex, FCommandListPool* pCmdListPool, ID3D12CommandQueue* pCmdQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE hRTV, D3D12_CPU_DESCRIPTOR_HANDLE hDSV, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	void Reset();

private:
	void CleanUp();

	const RenderItem_t* Dispatch();

private:
	FRenderer* _pRenderer = nullptr;
	AkU8* _pBuffer = nullptr;
	AkU32 _uMaxBufferSize = 0;
	AkU32 _uAllocatedSize = 0;
	AkU32 _uReadBufferPos = 0;
	AkU32 _uItemCount = 0;
};

