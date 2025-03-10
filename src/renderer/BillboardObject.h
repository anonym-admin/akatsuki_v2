#pragma once

/*
=================
BillboardObject
=================
*/

class FRenderer;

class FBillboardObjects : public IBillboard
{
public:
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 2;
	static const AkU32 DESCRIPTOR_COUNT_PER_BILLBOARD = 1;

	FBillboardObjects();
	~FBillboardObjects();

	AkBool Initialize(FRenderer* pRenderer);
	virtual AkBool CreateBillboardBuffer(const BillboardVertex_t* pBillboardVertices, AkU32 uPointNum)  override;
	virtual void SetTextureArray(void* pTexHandle) override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;
	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);

private:
	void CleanUp();
	AkBool CreateCommonResources();
	AkBool CreateRootSignature();
	AkBool CreatePipelineState();
	void DestroyCommonResources();
	void DestroyRootSignature();
	void DestroyPipelineState();

private:
	static ID3D12RootSignature* sm_pRootSignature;
	static ID3D12PipelineState* sm_pBillboardPSO;
	static AkU32 sm_uInitRefCount;
	AkU32 _uRefCount = 1;
	FRenderer* _pRenderer = nullptr;
	TextureHandle_t* _pTexArray = nullptr;
	ID3D12Resource* _pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW _tVertexBufferView = {};
	AkU32 _uPointNum = 0;
};

