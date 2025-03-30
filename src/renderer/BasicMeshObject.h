#pragma once

/*
===================
BasicMeshObject
===================
*/

class FRenderer;

class FBasicMeshObject : public IMeshObject
{
public:
    static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = 2;
    static const AkU32 DESCRIPTOR_COUNT_PER_MESH = 7 + 3 + 1 + 5; // albedo, normal ... height (t0~t6) / IBL (t11, t12, t13) / material cb (b2) / Shadow Map (t15, t16, 17)
    static const AkU32 MAX_MESH_COUNT_PER_OBJ = 8;
    static const AkU32 MAX_DESCRIPTOR_COUNT_FOR_DRAW = DESCRIPTOR_COUNT_PER_OBJ + (DESCRIPTOR_COUNT_PER_MESH * MAX_MESH_COUNT_PER_OBJ);

    FBasicMeshObject();
    ~FBasicMeshObject();

    AkBool Initialize(FRenderer* pRenderer);
    void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);
    void DrawNormal(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);
    void DrawShadowMaps(ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);
    void DrawReflection(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat);

    virtual AkBool CreateMeshBuffers(MeshData_t* pMeshData, AkU32 uMeshDataNum) override;
    virtual void* CreateDynamicMeshBuffers(Vertex_t* pVertices, AkU32 uVerticeNum, AkU32* pIndices, AkU32 uIndiceNum) override;
    virtual AkBool UpdateMaterialBuffers(const Vector3* pAlbedoFactor, AkF32 fMetallicFactor, AkF32 fRoughnessFactor, const Vector3* pEmisiionFactor) override;
    virtual void SetTextures(void* pAlbedo, void* pEmissve, void* pHeight, void* pNormal, void* pMetallic, void* pRoughness, void* pAO) override;
    virtual void EnableWireFrame() override { _bIsWire = AK_TRUE; }
    virtual void DisableWireFrame() override { _bIsWire = AK_FALSE; }
    virtual void DestoryDynamicVertexBuferHandle(void* pDVHandle) override;
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;

protected:
    void CleanUp();
    virtual void CreateVertexAndIndexBuffer(MeshData_t* pMeshData, AkU32 uMeshDataIndex);
    DynamicVertexBufferHandle_t* CreateDynamicVertexAndIndexBuffer(Vertex_t* pVertices, AkU32 uVerticeNum, AkU32* pIndices, AkU32 uIndiceNum);
    virtual AkBool CreateCommonResources();
    virtual AkBool CreateRootSignature();
    virtual AkBool CreatePipelineState();
    virtual void DestroyCommonResources();
    virtual void DestroyRootSignature();
    virtual void DestroyPipelineState();

private:
    static ID3D12RootSignature* sm_pRootSignature;
    static ID3D12PipelineState* sm_pBasicSolidPSO;
    static ID3D12PipelineState* sm_pBasicWirePSO;
    static ID3D12PipelineState* sm_pNormalPSO;
    static ID3D12PipelineState* sm_pDepthOnlyPSO;
    static ID3D12PipelineState* sm_pDrawMaskedSolidPSO;

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
};

