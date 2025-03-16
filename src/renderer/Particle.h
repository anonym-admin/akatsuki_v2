#pragma once

/*
==============
Particle
==============
*/

class FRenderer;

class FParticle : public IParticle
{
public:
	FParticle();
	~FParticle();

	AkBool Initialize(FRenderer* pRenderer);
	virtual AkBool CreateBasicParticleBuffer(VertexSize_t* pVertices, AkU32 uVerticeNum) override; // Paricle Count
	virtual void SetTexture(void* pTexHandle) override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList);

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
	static ID3D12PipelineState* sm_pParticlePSO;
	static AkU32 sm_uInitRefCount;
	AkU32 _uRefCount = 1;
	FRenderer* _pRenderer = nullptr;
	ID3D12Resource* _pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW _tVertexBufferView = {};
	AkU32 _uPointNum = 0;
	TextureHandle_t* _pTextureHandle = nullptr;
};

