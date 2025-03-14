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
	virtual AkBool CreateParticleBuffer(VertexSize_t* pVertices, AkU32 uVerticeNum) override; // Paricle Count
	virtual void SetTexture(const wchar_t* wcFilaname) override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList);

private:
	void CleanUp();

private:
	static ID3D12RootSignature* sm_pRootSignature;
	static ID3D12PipelineState* sm_pParticlePSO;
	FRenderer* _pRenderer = nullptr;
};

