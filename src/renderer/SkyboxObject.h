#pragma once

#include "interface/IRenderer.h"

/*
================
SkyboxObject
================
*/

class FRenderer;

class FSkyboxObject : public ISkyboxObject
{
public:
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 1;
	static const AkU32 DESCRIPTOR_COUNT_PER_MESH = 3;
	static const AkU32 MAX_MESH_COUNT_PER_OBJ = 1;
	static const AkU32 MAX_DESCRIPTOR_COUNT_FOR_DRAW = DESCRIPTOR_COUNT_PER_OBJ + (DESCRIPTOR_COUNT_PER_MESH * MAX_MESH_COUNT_PER_OBJ);

	FSkyboxObject();
	virtual ~FSkyboxObject();

	AkBool Initialize(FRenderer* pRenderer);
	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat, TextureHandle_t* pEnvHDR, TextureHandle_t* pDiffuseHDR, TextureHandle_t* pSpecularHDR);

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

private:
	void CleanUp();

	AkBool CreateCommonResources();
	AkBool CreateRootSignature();
	AkBool CreatePipelineState();
	AkBool CreateMesh();
	void DestroyCommonResources();
	void DestroyRootSignature();
	void DestroyPipelineState();
	void DestroyMesh();

private:
	FRenderer* _pRenderer = nullptr;

	// shared variable.
	static AkU32 sm_uInitRefCount;
	static ID3D12RootSignature* sm_pRootSignature;
	static ID3D12PipelineState* sm_pSkyboxPSO;

	// vertex data
	static ID3D12Resource* sm_pVertexBuffer;
	static D3D12_VERTEX_BUFFER_VIEW sm_tVertexBufferView;

	// index data
	static ID3D12Resource* sm_pIndexBuffer;
	static D3D12_INDEX_BUFFER_VIEW sm_tIndexBufferView;

	AkU32 _uRefCount = 1;

	// texture hnadle.
	TextureHandle_t* _pEnvHDR = nullptr;
	TextureHandle_t* _pDiffuseHDR = nullptr;
	TextureHandle_t* _pSpecularHDR = nullptr;
};

