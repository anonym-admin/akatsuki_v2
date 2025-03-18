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
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 4;
	static const AkU32 MAX_PARTICLE_COUNT = 1000;
	static const AkU32 STRUCTURED_BUFFER_COUNT = 1;
	static const AkU32 MAX_DESCRIPTOR_COUNT = DESCRIPTOR_COUNT_PER_OBJ + MAX_PARTICLE_COUNT + STRUCTURED_BUFFER_COUNT;

	FParticle();
	~FParticle();

	AkBool Initialize(FRenderer* pRenderer);
	virtual void* CreateBasicParticleBuffer(VertexParticle_t* pParticles, AkU32 uParticleNum) override; 
	virtual void SetTexture(void* pTexHandle) override;
	virtual void DestroyBasicParticleBuffer(void* pDBHandle) override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldRow, DynamicDefaultBufferHandle_t* pDBHandle, AkU32 uParticleNum, AkF32 fTime, AkF32 fDuration, const Vector2* pStartSize, const Vector3* pStartDirection, AkF32 fSizeOverLifeTime, const Vector3* pRotOverLifeTime, const Vector4* pTotalColor, const Vector4* pColorOverLifeTime);

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
	static ID3D12PipelineState* sm_pAccumulateParticlePSO;
	static AkU32 sm_uInitRefCount;
	AkU32 _uRefCount = 1;
	FRenderer* _pRenderer = nullptr;
	AkU32 _uParticleNum = 0;
	TextureHandle_t* _pTextureHandle = nullptr;
};

