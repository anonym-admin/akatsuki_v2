#pragma once

class FRenderer;

class FLineObject : public ILineObject
{
public:
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 2;

	FLineObject();
	~FLineObject();

	AkBool Initialize(FRenderer* pRenderer);
	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);

	virtual AkBool CreateLineBuffers(LineVertex_t* pStart, LineVertex_t* pEnd) override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;

private:
	void CleanUp();

	virtual AkBool CreateCommonResources();
	virtual AkBool CreateRootSignature();
	virtual AkBool CreatePipelineState();
	virtual void DestroyCommonResources();
	virtual void DestroyRootSignature();
	virtual void DestroyPipelineState();

private:
	static AkU32 sm_uInitRefCount;
	static ID3D12RootSignature* sm_pRootSignature;
	static ID3D12PipelineState* sm_pLinePSO;
	AkU32 _uRefCount = 1;
	FRenderer* _pRenderer = nullptr;

	// vertex data
	ID3D12Resource* _pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _tVertexBufferView;
	AkU32 _uVertexCount = 0;
};

