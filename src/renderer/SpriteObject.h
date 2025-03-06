#pragma once

enum class SPRITE_DESCRIPTOR_INDEX
{
	SPRITE_DESCRIPTOR_INDEX_CBV = 0,
	SPRITE_DESCRIPTOR_INDEX_TEX = 1
};

/*
===============
SpriteObject
===============
*/

class FRenderer;

class FSpriteObject : public ISpriteObject
{
public:
	static const AkU32 DESCRIPTOR_COUNT_FOR_DRAW = 2;	// | Constant Buffer | Tex |

	FSpriteObject();
	~FSpriteObject();

public:
	virtual void SetDrawBackground(AkBool bDrawBackground) override { _bDrawBackground = bDrawBackground; }
	// derived from IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	AkBool Initialize(FRenderer* pRenderer);
	AkBool Initialize(FRenderer* pRenderer, const wchar_t* wcTexFileName, const RECT* pRect);
	void DrawWithTex(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Vector2* pPos, const Vector2* pScale, const RECT* pRect, float Z, TextureHandle_t* pTexHandle, const Vector3* pFontColor);
	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Vector2* pPos, const Vector2* pScale, float Z);

private:
	void CleanUp();

	AkBool CreateCommonResources();
	AkBool CreateRootSignature();
	AkBool CreatePipelineState();
	AkBool CreateMesh();
	void DestroyCommonResources();

private:
	// shared variable.
	static ID3D12RootSignature* sm_pRootSignature;
	static ID3D12PipelineState* sm_pDefaultPSO;
	static ID3D12PipelineState* sm_pAccumulatePSO;
	static AkU32 sm_uInitRefCount;

	// vertex data.
	static ID3D12Resource* sm_pVertexBuffer;
	static D3D12_VERTEX_BUFFER_VIEW sm_tVertexBufferView;

	// index data.
	static ID3D12Resource* sm_pIndexBuffer;
	static D3D12_INDEX_BUFFER_VIEW sm_tIndexBufferView;

	AkU32 _uRefCount = 1;
	TextureHandle_t* _pTexHandle = nullptr;
	FRenderer* _pRenderer = nullptr;
	RECT _vRect = {};
	Vector2	_vScale = { 1.0f, 1.0f };
	AkBool _bDrawBackground = AK_FALSE;
};

