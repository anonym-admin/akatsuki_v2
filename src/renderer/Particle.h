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
	static const AkU32 SPARK_DESCRIPTOR_COUNT_PER_OBJ = 4;
	static const AkU32 SPRITE_DESCRIPTOR_COUNT_PER_OBJ = 3;
	static const AkU32 MAX_PARTICLE_COUNT = 1000;
	static const AkU32 MAX_DESCRIPTOR_COUNT = SPARK_DESCRIPTOR_COUNT_PER_OBJ + MAX_PARTICLE_COUNT;

	FParticle();
	~FParticle();

	AkBool Initialize(FRenderer* pRenderer);
	virtual void* CreateParticleSpark(VertexParticle_t* pParticles, AkU32 uParticleNum) override; 
	virtual void* CreateParticleSprite(VertexSize_t* pVerices) override;
	virtual void SetTexture(void* pTexHandle) override;
	virtual void DestroyBasicParticleBuffer(void* pDBHandle) override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldRow, DynamicDefaultBufferHandle_t* pDBHandle, AkU32 uParticleNum, AkF32 fTime, AkF32 fDuration, const Vector2* pStartSize, const Vector3* pStartDirection, AkF32 fSizeOverLifeTime, const Vector3* pRotOverLifeTime, const Vector4* pTotalColor, const Vector4* pColorOverLifeTime);
	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, DynamicDefaultBufferHandle_t* pDBHandle, const Vector2* pMaxFrame, const Vector2* pCurFrame);

private:
	void CleanUp();
	
	AkBool CreateCommonResources();
	AkBool CreateRootSignature();
	AkBool CreatePipelineState();
	void DestroyCommonResources();
	void DestroyRootSignature();
	void DestroyPipelineState();

private:
	static ID3D12RootSignature* sm_pSparkRS;
	static ID3D12RootSignature* sm_pSpriteRS;
	static ID3D12PipelineState* sm_pSparkPSO;
	static ID3D12PipelineState* sm_pSpritePSO;
	static AkU32 sm_uInitRefCount;
	AkU32 _uRefCount = 1;
	FRenderer* _pRenderer = nullptr;
	TextureHandle_t* _pTextureHandle = nullptr;
};

