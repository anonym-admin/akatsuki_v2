#pragma once

/*
=======================
LandScapeObject
=======================
*/

class FRenderer;

class FTerrainObject : public ITerrain
{
public:
    static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 3;
    static const AkU32 DESCRIPTOR_COUNT_PER_MESH = 6 + 3 + 1 + 5 + 2; // albedo, normal ... (t0~t5) / IBL (t11, t12, t13) / material cb (b2) / Shadow Map (t15, t16, 17) / t6, t7
    static const AkU32 MAX_MESH_COUNT_PER_OBJ = 1;
    static const AkU32 MAX_DESCRIPTOR_COUNT_FOR_DRAW = DESCRIPTOR_COUNT_PER_OBJ + (DESCRIPTOR_COUNT_PER_MESH * MAX_MESH_COUNT_PER_OBJ);

    FTerrainObject();
    ~FTerrainObject();

    AkBool Initialize(FRenderer* pRenderer);
    void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat, void* pBrush);
    void DrawNormal(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);
    void DrawShadow(ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);

    virtual AkBool CreateStaticMeshBuffers(VertexNormalTexcoordTangentAlpha_t* pVertices, AkU32 uVerticeNum, AkU32* pIndices, AkU32 uIndiceNum) override;
    virtual void* CreateDynamicMeshBuffers(VertexNormalTexcoordTangentAlpha_t* pVertices, AkU32 uVerticeNum, AkU32* pIndices, AkU32 uIndiceNum) override;
    virtual void SetTextures(const wchar_t* wcSecondFilename, const wchar_t* wcThirdFilename, const wchar_t* wcAlbedoFilename, const wchar_t* wcNormalFilename, const wchar_t* wcEmissvieFilename, const wchar_t* wcMetallicFilename, const wchar_t* wcRoughnessFilename, const wchar_t* wcAOFilename) override;
    virtual AkBool UpdateMaterialBuffers(const Vector3* pAlbedoFactor, AkF32 fMetallicFactor, AkF32 fRoughnessFactor, const Vector3* pEmisiionFactor) override;
    virtual void EnableWireFrame() override { _bIsWire = AK_TRUE; }
    virtual void DisableWireFrame() override { _bIsWire = AK_FALSE; }
    virtual void DestoryDynamicVertexBuferHandle(void* pDVHandle);
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;

private:
    void CleanUp();
    DynamicVertexHandle_t* CreateDynamicVertexAndIndexBuffer(VertexNormalTexcoordTangentAlpha_t* pVertices, AkU32 uVerticeNum, AkU32* pIndices, AkU32 uIndiceNum);
    AkBool CreateCommonResources();
    AkBool CreateRootSignature();
    AkBool CreatePipelineState();
    void DestroyCommonResources();
    void DestroyRootSignature();
    void DestroyPipelineState();

    void DeleteTextures();

private:
    static ID3D12RootSignature* sm_pRootSignature;
    static ID3D12PipelineState* sm_pBasicSolidPSO;
    static ID3D12PipelineState* sm_pBasicWirePSO;
    static ID3D12PipelineState* sm_pNormalPSO;
    static ID3D12PipelineState* sm_pDepthOnlyPSO;

protected:
    static AkU32 sm_uInitRefCount;
    AkU32 _uRefCount = 1;
    FRenderer* _pRenderer = nullptr;
    Mesh_t* _pMeshes = nullptr;
    AkU32 _uMeshNum = 0;
    AkBool _bIsWire = AK_FALSE;

    TextureHandle_t* _pIrradianceIBLTexHandle = nullptr;
    TextureHandle_t* _pSpecularIBLTexHandle = nullptr;
    TextureHandle_t* _pBrdfTexHandle = nullptr;

    MaterialConstantBuffer_t* _pMaterials = nullptr;

    TextureHandle_t* _pSecondTexHandle = nullptr;
    TextureHandle_t* _pThirdTexHandle = nullptr;
};

