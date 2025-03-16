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
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 2;
	static const AkU32 DESCRIPTOR_COUNT_PER_PARTICLES = 1; // Structured Buffer.
	static const AkU32 MAX_DESCRIPTOR_COUNT = DESCRIPTOR_COUNT_PER_OBJ + DESCRIPTOR_COUNT_PER_PARTICLES;

	FParticle();
	~FParticle();

	AkBool Initialize(FRenderer* pRenderer);
	virtual void* CreateBasicParticleBuffer(Particle_t* pParticles, AkU32 uParticleNum) override; 
	virtual void SetTexture(void* pTexHandle) override;
	virtual void DestroyBasicParticleBuffer(void* pDBHandle) override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, DynamicDefaultBufferHandle_t* pDBHandle);

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
	static ID3D12PipelineState* sm_pAccumulatePSO;
	static AkU32 sm_uInitRefCount;
	AkU32 _uRefCount = 1;
	FRenderer* _pRenderer = nullptr;
	AkU32 _uParticleNum = 0;
	TextureHandle_t* _pTextureHandle = nullptr;

	ID3D12Resource* _pInputBufferA = nullptr;
	ID3D12Resource* _pInputBufferB = nullptr;
	ID3D12Resource* _pOutputBuffer = nullptr;
	ID3D12Resource* _pReadBackBuffer = nullptr;

	static const AkU32 NUM_DATA = 32;
};

