#pragma once

class FRenderer;

class FCloudObject : public IEnvironmentObject
{
public:
    static const AkU32 DESCRIPTOR_COUNT_DENSITY_MAP = 2;
    static const AkU32 DESCRIPTOR_COUNT_LIGHT_MAP = 3;
    static const AkU32 DESCRIPTOR_COUNT_VOLUME = 6;

    FCloudObject();
    ~FCloudObject();

    AkBool Initialize(FRenderer* pRenderer);
    void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, AkF32 fTime, const Matrix* pWorldRow, AkF32 fLightAbsorptionCoeff, const Vector3* pLightDir, AkF32 fDensityAbsorption, const Vector3* pLightColor, AkF32 fAniso);
    virtual AkBool CreateMeshBuffers(MeshData_t* pMeshData, AkU32 uMeshDataNum) override;
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;

private:
    void CleanUp();

    AkBool CreateCommonResources();
    AkBool CreateRootSignature();
    AkBool CreatePipelineState();
    AkBool CreateDensityAndLightMap();
    void DestroyCommonResources();
    void DestroyRootSignature();
    void DestroyPipelineState();
    void DestroyDensityAndLightMap();

private:
    static ID3D12RootSignature* sm_pCloudDensityRS;
    static ID3D12RootSignature* sm_pCloudLightRS;
    static ID3D12RootSignature* sm_pVolumeRS;
    static ID3D12PipelineState* sm_pCloudDensityPSO;
    static ID3D12PipelineState* sm_pCloudLightPSO;
    static ID3D12PipelineState* sm_pVolumePSO;
    static AkU32 sm_uInitRefCount;

    AkU32 _uRefCount = 1;
    FRenderer* _pRenderer = nullptr;
    Mesh_t* _pMeshes = nullptr;
    AkU32 _uMeshNum = 0;

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
    D3D12_CPU_DESCRIPTOR_HANDLE _hNullCpu = {};

    ENVIRONMENT_TYPE _eType = CLOUD;
};