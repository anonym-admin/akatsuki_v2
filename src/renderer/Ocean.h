#pragma once

/*
================
Ocean
================
*/

class FRenderer;

class FOceanObject : public IEnvironmentObject
{
public:
    static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 2;

	FOceanObject();
	~FOceanObject();

	AkBool Initialize(FRenderer* pRenderer);
	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, AkF32 fTime, const Matrix* pWorldMat);
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;

private:
    void CleanUp();

    AkBool CreateCommonResources();
    AkBool CreateRootSignature();
    AkBool CreatePipelineState();
    AkBool CreateMeshBuffers();
    void DestroyCommonResources();
    void DestroyRootSignature();
    void DestroyPipelineState();
    void DestroyMeshBuffers();

private:
    static ID3D12RootSignature* sm_pRootSignature;
    static ID3D12PipelineState* sm_pOceanPSO;
    static AkU32 sm_uInitRefCount;

    AkU32 _uRefCount = 1;
    FRenderer* _pRenderer = nullptr;
    Mesh_t* _pMesh = nullptr;
};



