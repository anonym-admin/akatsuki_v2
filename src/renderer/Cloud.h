#pragma once

class FRenderer;

class FCloudObject : public IEnvironmentObject
{
public:
    static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 2;

    FCloudObject();
    ~FCloudObject();

    AkBool Initialize(FRenderer* pRenderer);
    void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList);
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;

private:
    void CleanUp();

    AkBool CreateCommonResources();
    AkBool CreateRootSignature();
    AkBool CreatePipelineState();
    AkBool CreateBuffers();
    void DestroyCommonResources();
    void DestroyRootSignature();
    void DestroyPipelineState();
    void DestroyBuffers();

private:
    static ID3D12RootSignature* sm_pRootSignature;
    static ID3D12PipelineState* sm_pCloudDensityPSO;
    static ID3D12PipelineState* sm_pCloudLightPSO;
    static AkU32 sm_uInitRefCount;

    AkU32 _uRefCount = 1;
    FRenderer* _pRenderer = nullptr;
    Mesh_t* _pMesh = nullptr;

    DXGI_FORMAT _tFormat = DXGI_FORMAT_R16_FLOAT;
    AkU32 _uVolumeWidth = 128;
    AkU32 _uVolumeHeight = 128;
    AkU32 _uVolumeDepth = 128;
    AkU32 _uLightWidth = 32;
    AkU32 _uLightHeight = 32;
    AkU32 _uLightDepth = 32;

    ID3D12Resource* _pDensityMap = nullptr;
    ID3D12Resource* _pLightMap = nullptr;

    D3D12_CPU_DESCRIPTOR_HANDLE _hDensitySRVCpu = {};
    D3D12_CPU_DESCRIPTOR_HANDLE _hDensityUAVCpu = {};
    D3D12_CPU_DESCRIPTOR_HANDLE _hLightSRVCpu = {};
    D3D12_CPU_DESCRIPTOR_HANDLE _hLightUAVCpu = {};

    ENVIRONMENT_TYPE _eType = CLOUD;
};