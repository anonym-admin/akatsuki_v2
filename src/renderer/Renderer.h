#pragma once

#define MULTI_THREAD_RENDERING

/*
=======================
Render core class
=======================
*/

class FDescriptorAllocator;
class FDescriptorPool;
class FCommandListPool;
class FConstantBufferPool;
class FConstantBufferManager;
class FResourceManager;
class FTextureManager;
class FFontManager;
class FRenderQueue;
class FPostEffect;
class FPostProcess;
class FRenderTransparent;
class FRenderMirror;
class FRenderUI;

class FRenderDepthMap;
class FRenderShadowMap;

class FRenderer : public IRenderer
{
public:
	static const AkU32 MAX_DRAW_COUNT_PER_FRAME = 4096;
	static const AkU32 MAX_DESCRIPTOR_COUNT = 4096;
	static const AkU32 MAX_RENDER_THREAD_COUNT = 8;
	static const AkU32 CASCADE_SHADOW_MAP_LEVEL = 5;
	static const AkU32 FLOAT16_BUFFER_COUNT = 1;
	static const AkU32 RESOLVED_BUFFER_COUNT = 1;
	static const AkU32 POST_EFFECT_BUFFER_COUNT = 1;
	static const AkU32 MAX_FRAME_BUFFER_COUNT = FLOAT16_BUFFER_COUNT + RESOLVED_BUFFER_COUNT + POST_EFFECT_BUFFER_COUNT;

	FRenderer();
	~FRenderer();

	/*interface*/
	virtual AkBool Initialize(HWND hWnd, AkBool bEnableDebugLayer, AkBool bEnableGBV) override;
	virtual void BeginRender() override;
	virtual void EndRender() override;
	virtual void BeginRenderDepthMap() override;
	virtual void EndRenderDepthMap() override;
	virtual void BeginRenderShadowMaps() override;
	virtual void EndRenderShadowMaps() override;
	virtual void Present() override;
	virtual IMeshObject* CreateBasicMeshObject() override;
	virtual IMeshObject* CreateSkinnedMeshObject() override;
	virtual ISprite* CreateSpriteObject() override;
	virtual ISprite* CreateSpriteObjectWidthTex(const wchar_t* wcTexFilename, AkI32 iPosX, AkI32 iPosY, AkI32 iWidth, AkI32 iHeight) override;
	virtual ISkybox* CreateSkyboxObject() override;
	virtual ILineObject* CreateLineObject() override;
	virtual IBillboard* CreateBillboard() override;
	virtual ITerrain* CreateTerrain() override;
	virtual IParticle* CreateParticle() override;
	virtual IEnvironmentObject* CreateOceanObject() override;
	virtual IEnvironmentObject* CreateCloudObject() override;
	virtual void* CreateTextureFromFile(const wchar_t* wcFilename, AkBool bUseSRGB, AkBool bIsArray = AK_FALSE) override;
	virtual void* CreateCubeMapTexture(const wchar_t* wcFilename) override;
	virtual void* CreateDynamicTexture(AkU32 uTexWidth, AkU32 uTexHeight) override;
	virtual void* CreateFontObject(const wchar_t* wcFontFamilyName, AkF32 fFontSize) override;
	virtual void BindIBLTexture(void* pIrradianceTexHandle, void* pSpecularTexHandle, void* pBrdfTexHandle) override;
	virtual void BindImGui(void** ppImGuiCtx) override;
	virtual void UnBindImGui() override;
	virtual AkBool WriteTextToBitmap(AkU8* pDestImage, AkU32 uDestWidth, AkU32 uDestHeight, AkU32 uDestPitch, AkI32* pWidth, AkI32* pHeight, void* pFontHandle, const wchar_t* wcText, AkU32 uTextLength, FONT_COLOR_TYPE eFontColor) override;
	virtual AkBool UpdateWindowSize(AkU32 uScreenWidth, AkU32 uScreenHeight) override;
	virtual void UpdateTextureWidthImage(void* pTexHandle, const AkU8* pSrcImage, AkU32 uSrcWidth, AkU32 uSrcHeight) override;
	virtual void UpdateCascadeOrthoProjMatrix() override;
	virtual void UpdateDynamicDefaultBuffer(void* pDBHandle, const void* pData) override;
	virtual void UpdateDynamicVertexBuffer(void* pDVHandle, const void* pData) override;
	virtual void DestroyTexture(void* pTexHandle) override;
	virtual void DestroyFontObject(void* pFontHandle) override;
	virtual void DestroyDynamicVertex(void* pDVHandle) override;
	virtual void RenderBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) override;
	virtual void RenderNormalOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) override;
	virtual void RenderDepthMapOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) override;
	virtual void RenderShadowOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) override;
	virtual void RenderSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) override;
	virtual void RenderNormalOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) override;
	virtual void RenderDepthMapOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) override;
	virtual void RenderShadowOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) override;
	virtual void RenderSpriteWithTex(void* pSpriteObjHandle, AkI32 iPosX, AkI32 iPosY, AkF32 fScaleX, AkF32 fScaleY, const RECT* pRect, AkF32 fZ, void* pTexHandle, AkBool bUseBlend) override;
	virtual void RenderSprite(void* pSpriteObjHandle, AkI32 iPosX, AkI32 iPosY, AkF32 fScaleX, AkF32 fScaleY, AkF32 fZ, AkBool bUseBlend) override;
	virtual void RenderSkybox(ISkybox* pSkyboxObj, const Matrix* pWorldMat, void* pEnvHDR, void* pDiffuseHDR, void* pSpecularHDR) override;
	virtual void RenderLineObject(ILineObject* pLineObj, const Matrix* pWorldMat) override;
	virtual void RenderBillboard(IBillboard* pBillboard, const Matrix* pWorldMat) override;
	virtual void RenderDepthMapOfBillboard(IBillboard* pBillboard, const Matrix* pWorldMat) override;
	virtual void RenderShadowOfBillboard(IBillboard* pBillboard, const Matrix* pWorldMat) override;
	virtual void RenderTerrain(ITerrain* pTerrain, const Matrix* pWorldMat, void* pBrush) override;
	virtual void RenderNormalOfTerrain(ITerrain* pTerrain, const Matrix* pWorldMat, void* pBrush) override;
	virtual void RenderDepthMapOfTerrain(ITerrain* pTerrain, const Matrix* pWorldMat) override;
	virtual void RenderShadowOfTerrain(ITerrain* pTerrain, const Matrix* pWorldMat) override;
	virtual void RenderParticleSpark(IParticle* pParticle, const Matrix* pWorldRow, void* pDBHandle, AkU32 uParticleNum, AkF32 fTime, AkF32 fDuration, const Vector2* pStartSize, const Vector3* pStartDirection, AkF32 fSizeOverLifeTime, const Vector3* pRotOverLifeTime, const Vector4* pTotalColor, const Vector4* pColorOverLifeTime) override;
	virtual void RenderParticleSprite(IParticle* pParticle, void* pDBHandle, const Vector2* pMaxFrame, const Vector2* pCurFrame) override;
	virtual void RenderOcean(IEnvironmentObject* pOcean, AkF32 fTime, const Matrix* pWorldMat) override;
	virtual void RenderCloud(IEnvironmentObject* pCloud, AkF32 fTime, const Matrix* pWorldRow, AkF32 fLightAbsorptionCoeff, const Vector3* pLightDir, AkF32 fDensityAbsorption, const Vector3* pLightColor, AkF32 fAniso) override;
	virtual void RenderReflectionOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) override;
	virtual void RenderReflectionOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBoneTransform) override;
	virtual void RenderReflectionOfTerrain(ITerrain* pTerrain, const Matrix* pWorldMat) override;
	virtual void RenderReflectionOfSkybox(ISkybox* pSkybox, const Matrix* pWorldMat, void* pEnvHDR, void* pDiffuseHDR, void* pSpecularHDR) override;
	virtual void RotateXCamera(AkF32 fRadian) override;
	virtual void RotateYCamera(AkF32 fRadian) override;
	virtual void RotateYawPitchRollCamera(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll) override;
	virtual void MoveCamera(AkF32 fX, AkF32 fY, AkF32 fZ) override;
	virtual void AddGlobalLight(const Vector3* pRadiance, const Vector3* pDir, AkBool bShadow) override;
	virtual void AddPointLight(const Vector3* pRadiance, const Vector3* pPos, AkF32 fRadius, AkF32 fFallOffStart, AkF32 fFallOffEnd, AkBool bShadow) override;
	virtual void AddSpotLight(const Vector3* pRadiance, const Vector3* pPos, const Vector3* pDir, AkF32 fRadius, AkF32 fFallOffStart, AkF32 fFallOffEnd, AkF32 fSpotPower, AkBool bShadow) override;
	virtual void UpdatePointLight(AkU32 uIndex, const Vector3* pRadiance, const Vector3* pPos, AkF32 fRadius, AkF32 fFallOffStart, AkF32 fFallOffEnd, AkBool bShadow) override;
	virtual void UpdateSpotLight(AkU32 uIndex, const Vector3* pRadiance, const Vector3* pPos, const Vector3* pDir, AkF32 fRadius, AkF32 fFallOffStart, AkF32 fFallOffEnd, AkF32 fSpotPower, AkBool bShadow) override;
	virtual void SetCameraPosition(AkF32 fX, AkF32 fY, AkF32 fZ) override;
	virtual void SetCamera(const Vector3* pCamPos, const Vector3* pCamDir, Vector3* pCamUp) override;
	virtual void SetGlobalIBLStrength(AkF32 fIBLStrength) override;
	virtual void SetFogStrength(AkF32 fFogStrength) override;
	virtual void SetDepthScale(AkF32 fDepthScale) override;
	virtual void SetPostEffectMode(AkI32 iMode) override;
	virtual void SetVSync(AkBool bUseVSync) override;
	virtual void SetToneMappingType(AkI32 iType) override { _iToneMappingType = iType; }
	virtual void SetBloomLevels(AkU32 uLevel) override { _uBloomLevels = uLevel; }
	virtual void SetBloomStrength(AkF32 uStrength) override { _fBloomStrength = uStrength; }
	virtual void SetFullScreen(AkBool bIsFullScreen) override;
	virtual void SetTotalTime(AkF32 fTime) override;
	virtual void GetCameraPosition(AkF32* pX, AkF32* pY, AkF32* pZ) override;
	virtual Vector3 GetWorldNearPosition(AkF32 fNdcX, AkF32 fNdcY) override;
	virtual Vector3 GetWorldFarPosition(AkF32 fNdcX, AkF32 fNdcY) override;
	virtual void GetViewPorjMatrix(Matrix* pViewMat, Matrix* pProjMat) override;
	virtual void GetRelectionViewProjMatrix(Matrix* pViewMat, Matrix* pProjMat) override;
	virtual void GetFrustum(Vector4* pOutPlane) override;
	virtual AkBool MousePickingToPlane(DirectX::SimpleMath::Plane* pPlane, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) override;
	virtual AkBool MousePickingToTriangle(Vector3* pV0, Vector3* pV1, Vector3* pV2, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) override;
	virtual AkBool MousePickingToSqaure(Vector3* pV0, Vector3* pV1, Vector3* pV2, Vector3* pV3, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) override;
	virtual AkBool MousePickingToSphere(DirectX::BoundingSphere* pSphere, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) override;
	virtual AkBool MousePickingToBox(DirectX::BoundingBox* pBox, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) override;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	/*dll inner*/
	HWND GetHwnd() { return _hWnd; }
	ID3D12Device* GetDevice() { return _pDevice; }
	FDescriptorAllocator* GetDescriptorAllocator() { return _pDescriptorAllocator; }
	FDescriptorPool* GetDescriptorPool(AkU32 uThreadIndex) { return _ppDescriptorPool[_uCurContextIndex][uThreadIndex]; }
	FResourceManager* GetResourceManager() { return _pResourceManager; }
	FTextureManager* GetTextureManager() { return _pTextureManager; }
	FConstantBufferPool* GetConstantBufferPool(AkU32 uThreadIndex, CONSTANT_BUFFER_TYPE eConstBufType);
	AkU32 GetScreenWidth() { return _uScreenWidth; }
	AkU32 GetSreenHeight() { return _uScreenHeight; }
	AkF32 GetDpi() { return _fDpi; }
	void GetShadowViewProjMatrix(Matrix* pViewMat, Matrix* pProjMat, AkU32 uCascadeIndex);
	void GetIBLTexture(TextureHandle_t** ppOutIrradianceTexHandle, TextureHandle_t** ppOutSpecularTexHandle, TextureHandle_t** ppOutBrdfTexHandle);
	Light_t* GetLights(AkU32* pOutPointLightNum, AkU32* pOutSpotLightNum);
	AkF32 GetIBLStrength() { return _fIBLStrength; }
	void GetShadowMapSrv(D3D12_CPU_DESCRIPTOR_HANDLE* pOutHandle, AkU32 uCascadeIndex);
	AkU32 GetCascadeIndex() { return _uCascadeIndex; }
	DXGI_FORMAT GetBackBufferRTVFormat() { return _tBackBufferFormat; }
	DXGI_FORMAT GetFloatRTVFormat() { return _tFloatBufferFormat; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetBackBufferRtvCpu() { return CD3DX12_CPU_DESCRIPTOR_HANDLE(_pRTVHeap->GetCPUDescriptorHandleForHeapStart(), _uRTIndex, _uRTVDesciptorSize); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetFloatBufferSrvCpu() { return _hFloatBufferSrvCpu; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetResolvedBufferSrvCpu() { return _hResolvedBufferSrvCpu; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetPostEffectBufferrSrvCpu() { return _hPostEffectBufferSrvCpu; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthMapBufferSrvCpu() { return _hDepthMapSrvCpu; }
	ID3D12Resource* GetResolvedBuffer() { return _pResolvedBuffer; }
	ID3D12Resource* GetPostEffectBuffer() { return _pPostEffectBuffer; }
	ID3D12Resource* GetBackBuffer() { return _ppBackBuffer[_uRTIndex]; }
	ID3D12Resource* GetDepthMapBuffer() { return _pDepthOnlyDS; }
	ID3D12DescriptorHeap* GetRtvHeap() { return _pRTVHeap; }
	AkU32 GetRtvDescriptorSize() { return _uRTVDesciptorSize; }
	AkU32 GetDsvDescriptorSize() { return _uDSVDescriptorSize; }
	AkF32* GetRTVClearColor() { return _pRTVClearColor; }
	AkBool UseMSAA() { return _bUseMSAA; }
	AkU32 GetNumQualityLevel() { return _uNumQualityLevels; }
	AkU32 GetBloomLevel() { return _uBloomLevels; }
	AkI32 GetToneMappingType() { return _iToneMappingType; }
	AkF32 GetBloomStrength() { return _fBloomStrength; }
	AkF32 GetToltalTime() { return _fTotalTime; }

	void EnsureCompleted();

	void ProcessByThread(AkU32 uThreadIndex);

private:
	void CleanUp();

	AkBool CreateDevice(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter);
	AkBool CreateCmdQueue();
	AkBool CreateDescriptorForRTV();
	AkBool CreateDescriptorForDSV();
	AkBool CreateSwapChain(IDXGIFactory4* pFactory, AkU32 uScreenWidth, AkU32 uScreenHeight);
	AkBool CreateRTVs();
	AkBool CreateAdditionalRTVsAndSRVs();
	AkBool CreateDSVs(AkU32 uWidth, AkU32 uHeight);
	AkBool CreateShadowDSVs(AkU32 uWidth, AkU32 uHeight);
	AkBool CreateFence();
	AkBool CreateRenderThreadPool(AkU32 uThreadCount);
	AkBool CrreatePostEffect();
	AkBool CreatePostProcess();
	AkBool CreateImGuiInitResource();
	AkBool CreateRenderUI();
	AkBool CreateRenderParticle();
	AkBool CreateRenderMirror();

	void InitViewports(AkF32 fWidth, AkF32 fHeight);
	void InitScissorRect(AkU32 uWidth, AkU32 uHeight);
	void InitCamera();
	void DestroyDevice();
	void DestroyCmdQueue();
	void DestroyDescriptorForRTV();
	void DestroyDescriptorForDSV();
	void DestroySwapChain();
	void DestroyRTVs();
	void DestroyAdditionalRTVsAndSRVs();
	void DestroyDSVs();
	void DestroyShadowDSVs();
	void DestroyFence();
	void DestroyRenderThreadPool(AkU32 uThreadCount);
	void DestroyPostProcess();
	void DestroyPostEffect();
	void DestroyImGuiInitResource();
	void DestroyRenderUI();
	void DestroyRenderParticle();
	void DestroyRenderMirror();

	void Fence();
	void WaitForFenceValue(AkU64 u64ExpectedFenceValue);

	void CalculateMousePickingRayCast(AkF32 fNdcX, AkF32 fNdcY, DirectX::SimpleMath::Ray* pRay, AkF32* pRayLength);

private:
	HWND _hWnd = nullptr;
	AkU32 _uRefCount = 1;
	AkF32 _fDpi = 0.0f;
	AkF32 _fTotalTime = 0.0f;
	ID3D12Device* _pDevice = nullptr;
	ID3D12CommandQueue* _pCmdQueue = nullptr;
	ID3D12DescriptorHeap* _pRTVHeap = nullptr;
	ID3D12DescriptorHeap* _pDSVHeap = nullptr;
	ID3D12DescriptorHeap* _pImGuiHeap = nullptr;
	IDXGISwapChain3* _pSwapChain = nullptr;
	ID3D12Resource* _ppBackBuffer[SWAP_CHAIN_FRAME_COUNT];
	ID3D12Resource* _pMainDSwithMSAA = nullptr;
	ID3D12Resource* _pMainDS = nullptr;
	ID3D12Resource* _pDepthOnlyDS = nullptr;
	ID3D12Resource* _pShadowDS[CASCADE_SHADOW_MAP_LEVEL] = {};
	ID3D12Fence* _pFence = nullptr;
	D3D12_VIEWPORT _tMainViewport = {};
	D3D12_VIEWPORT _tShadowViewport = {};
	D3D12_RECT _tMainScissorRect = {};
	D3D12_RECT _tShadowScissorRect = {};
	DXGI_ADAPTER_DESC1 _tAdapterDesc = {};
	HANDLE _hFenceEvent = nullptr;
	FDescriptorAllocator* _pDescriptorAllocator = nullptr;
	FDescriptorPool* _ppDescriptorPool[PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	FCommandListPool* _ppCommandListPool[PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	FConstantBufferManager* _ppConstantBufferManger[PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	FResourceManager* _pResourceManager = nullptr;
	FTextureManager* _pTextureManager = nullptr;
	FFontManager* _pFontManager = nullptr;
	FRenderQueue* _ppRenderQueue[MAX_RENDER_THREAD_COUNT] = {};
	struct RENDER_THREAD_DESC* _pRenderThreadDescList = nullptr;
	HANDLE _hCompleteEvent = nullptr;
	Vector3 _vCamPos = Vector3(0.0f);
	Vector3 _vCamDir = Vector3(0.0f);
	Matrix _mViewMat = Matrix();
	Matrix _mProjMat = Matrix();
	AkBool _bUseImgui = AK_FALSE;
	AkU32 _uVSyncInterval = 1; // VSync Off.
	AkU32 _uRTIndex = 0;
	AkU32 _uRTVDesciptorSize = 0;
	AkU32 _uDSVDescriptorSize = 0;
	AkU32 _uSwapChainFlag = 0;
	AkU32 _uScreenWidth = 0;
	AkU32 _uScreenHeight = 0;
	AkU32 _uShadowWidth = 4096;
	AkU32 _uShadowHeight = 4096;
	AkU64 _u64FenceValue[PENDING_FRAME_COUNT] = {};
	AkU64 _u64CommonFenceVaule = 0;
	AkU32 _uRenderThreadCount = 0;
	AkU32 _uCurThreadIndex = 0;
	AkU32 _uCurContextIndex = 0;
	AkF32 _fFov = 0.0f;
	AkF32 _fNear = 0.1f;
	AkF32 _fFar = 1000.0f;
	AkBool _bFullScreen = AK_FALSE;
	AkF32 _pRTVClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	volatile AkU32 _uActiveThreadCount = 0;
	TextureHandle_t* _pIrradianceIBLTexHandle = nullptr;
	TextureHandle_t* _pSpecularIBLTexHandle = nullptr;
	TextureHandle_t* _pBrdfTexHandle = nullptr;
	Vector4 _pFrustumPoints[8] = {};
	Matrix _pShadowOrthoProj[CASCADE_SHADOW_MAP_LEVEL] = {};
	Matrix _pShadowView[CASCADE_SHADOW_MAP_LEVEL] = {};
	AkF32 _pCascadeBoundary[CASCADE_SHADOW_MAP_LEVEL + 1] = {};
	AkU32 _uCascadeIndex = 1;
	Light_t _tGlobalLight = {};
	Light_t _pPointLights[POINT_LIGHTS_NUM] = {};
	Light_t _pSpotLights[SPOT_LIGHTS_NUM] = {};
	Light_t _pLightList[MAX_LIGHTS_COUNT] = {};
	AkU32 _uPointLightsNum = 0;
	AkU32 _uSpotLightsNum = 0;
	Vector3 _vLightPos = Vector3(0.0f, 2.5f, 1025.0f);
	D3D12_CPU_DESCRIPTOR_HANDLE _hDepthMapSrvCpu = {};
	D3D12_CPU_DESCRIPTOR_HANDLE _pShadowMapSrvCpu[CASCADE_SHADOW_MAP_LEVEL] = {};

	ID3D12Resource* _pFloatBuffer = nullptr;
	ID3D12Resource* _pResolvedBuffer = nullptr;
	ID3D12Resource* _pPostEffectBuffer = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE _hFloatBufferSrvCpu = {};
	D3D12_CPU_DESCRIPTOR_HANDLE _hResolvedBufferSrvCpu = {};
	D3D12_CPU_DESCRIPTOR_HANDLE _hPostEffectBufferSrvCpu = {};
	DXGI_FORMAT _tBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT _tFloatBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	AkF32 _fIBLStrength = 1.0f;
	AkBool _bUseMSAA = AK_TRUE;
	AkU32 _uNumQualityLevels = 0;

	// Post Process Parameter
	AkI32 _iToneMappingType = 0; // 0 : Linear, 1 : Uncharted, 2: Filmic
	AkU32 _uBloomLevels = 4;
	AkF32 _fBloomStrength = 0.05f;

	// Rendering Pass
	FRenderDepthMap* _pRenderDepthMap = nullptr;
	FRenderShadowMap* _pRenderShadowMap = nullptr;
	FRenderTransparent* _pRenderTransparent = nullptr;
	FPostEffect* _pPostEffect = nullptr;
	FPostProcess* _pPostProcess = nullptr;
	FRenderUI* _pRenderUI = nullptr;

	DirectX::SimpleMath::Plane _tMirrorPlane = {};
	FRenderMirror* _pRenderMirror = nullptr;
};

