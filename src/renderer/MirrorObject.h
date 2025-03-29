#pragma once

#include "interface/IRenderer.h"

class FRenderer;

class FMirrorObject : public IMirror
{
public:
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 2;

	FMirrorObject();
	~FMirrorObject();

	AkBool Initialize(FRenderer* pRenderer);
	void Masking(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);
	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;
	
private:
	void CleanUp();

	AkBool CreateCommonResources();
	AkBool CreateRootSignature();
	AkBool CreatePipelineState();
	AkBool CreateMeshBuffers();
	virtual void DestroyCommonResources();
	virtual void DestroyRootSignature();
	virtual void DestroyPipelineState();
	void DestroyMeshBuffers();

private:
	static ID3D12RootSignature* sm_pRootSignature;
	static ID3D12PipelineState* sm_pStencilMaskPSO;    
	static AkU32 sm_uInitRefCount;
	static Mesh_t* sm_pMesh;

	AkU32 _uRefCount = 1;
	FRenderer* _pRenderer = nullptr;
};

