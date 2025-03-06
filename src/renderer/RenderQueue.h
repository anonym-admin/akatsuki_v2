#pragma once

enum class RENDER_ITEM_TYPE
{
	RENDER_ITEM_TYPE_MESH_OBJ,
	RENDER_ITEM_TYPE_SKINNED_MESH_OBJ,
	RENDER_ITEM_TYPE_SKYBOX_OBJ,
	RENDER_ITEM_TYPE_SPRITE_OBJ,
	RENDER_ITEM_TYPE_LINE_OBJ,
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
	const Vector3* pFontColor = nullptr;
};

struct RenderLineObjParam_t
{
	const Matrix* _pWorld = nullptr;
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

