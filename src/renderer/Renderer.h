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
class FPostProcess;

class FRenderer : public IRenderer
{
public:
	static const AkU32 MAX_DRAW_COUNT_PER_FRAME = 4096 * 2;
	static const AkU32 MAX_DESCRIPTOR_COUNT = 4096 * 2;
	static const AkU32 MAX_RENDER_THREAD_COUNT = 8;
	static const AkU32 CASCADE_SHADOW_MAP_LEVEL = 5;

	FRenderer();
	~FRenderer();

	/*interface*/
	virtual AkBool Initialize(HWND hWnd, AkBool bEnableDebugLayer, AkBool bEnableGBV) override;
	virtual void BeginRender() override;
	virtual void EndRender() override;
	virtual void BeginCasterRenderPreparation() override;
	virtual void EndCasterRenderPreparation() override;
	virtual void Present() override;
	virtual IMeshObject* CreateBasicMeshObject() override;
	virtual IMeshObject* CreateSkinnedMeshObject() override;
	virtual ISpriteObject* CreateSpriteObject() override;
	virtual ISpriteObject* CreateSpriteObjectWidthTex(const wchar_t* wcTexFilename, AkI32 iPosX, AkI32 iPosY, AkI32 iWidth, AkI32 iHeight) override;
	virtual ISkyboxObject* CreateSkyboxObject() override;
	virtual ILineObject* CreateLineObject() override;
	virtual void* CreateTextureFromFile(const wchar_t* wcFilename, AkBool bUseSRGB) override;
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
	virtual void DestroyTexture(void* pTexHandle) override;
	virtual void DestroyFontObject(void* pFontHandle) override;
	virtual void RenderBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) override;
	virtual void RenderNormalOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) override;
	virtual void RenderShadowOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) override;
	virtual void RenderSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) override;
	virtual void RenderNormalOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) override;
	virtual void RenderShadowOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) override;
	virtual void RenderSpriteWithTex(void* pSpriteObjHandle, AkI32 iPosX, AkI32 iPosY, AkF32 fScaleX, AkF32 fScaleY, const RECT* pRect, AkF32 fZ, void* pTexHandle, const Vector3* pFontColor) override;
	virtual void RenderSprite(void* pSpriteObjHandle, AkI32 iPosX, AkI32 iPosY, AkF32 fScaleX, AkF32 fScaleY, AkF32 fZ) override;
	virtual void RenderSkybox(ISkyboxObject* pSkyboxObj, const Matrix* pWorldMat, void* pEnvHDR, void* pDiffuseHDR, void* pSpecularHDR) override;
	virtual void RenderLineObject(ILineObject* pLineObj, const Matrix* pWorldMat) override;
	virtual void RotateXCamera(AkF32 fRadian) override;
	virtual void RotateYCamera(AkF32 fRadian) override;
	virtual void RotateYawPitchRollCamera(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll) override;
	virtual void MoveCamera(AkF32 fX, AkF32 fY, AkF32 fZ) override;
	virtual void AddGlobalLight(const Vector3* pRadiance, const Vector3* pDir, AkBool bShadow) override;
	virtual void AddPointLight(const Vector3* pPos, const Vector3* pDir, AkF32 fRadius, AkF32 fFallOffStart, AkF32 fFallOffEnd, AkF32 fSpotPower, AkBool bShadow) override;
	virtual void SetCameraPosition(AkF32 fX, AkF32 fY, AkF32 fZ) override;
	virtual void SetCamera(const Vector3* pCamPos, const Vector3* pCamDir, Vector3* pCamUp) override;
	virtual void SetIBLStrength(AkF32 fIBLStrength) override;
	virtual void SetVSync(AkBool bUseVSync) override;
	virtual void GetCameraPosition(AkF32* pX, AkF32* pY, AkF32* pZ) override;
	virtual Vector3 GetWorldNearPosition(AkF32 fNdcX, AkF32 fNdcY) override;
	virtual Vector3 GetWorldFarPosition(AkF32 fNdcX, AkF32 fNdcY) override;
	virtual void GetViewPorjMatrix(Matrix* pViewMat, Matrix* pProjMat) override;
	virtual void GetFrustum(AkPlane_t* ppOutPlane) override;
	virtual AkBool MousePickingToPlane(DirectX::SimpleMath::Plane* pPlane, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) override;
	virtual AkBool MousePickingToSphere(DirectX::BoundingSphere* pSphere, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) override;
	virtual AkBool MousePickingToBox(DirectX::BoundingBox* pBox, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) override;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	/*dll inner*/
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
	void GetLightPosition(AkF32* fX, AkF32* fY, AkF32* fZ);
	void GetIBLTexture(TextureHandle_t** ppOutIrradianceTexHandle, TextureHandle_t** ppOutSpecularTexHandle, TextureHandle_t** ppOutBrdfTexHandle);
	Light_t* GetLights(AkU32* pOutLightNum);
	AkF32 GetIBLStrength() { return _fIBLStrength; }
	void GetGlobalLight(Light_t* pOutLight);
	void GetShadowMapSrv(D3D12_CPU_DESCRIPTOR_HANDLE* pOutHandle, AkU32 uCascadeIndex);
	AkU32 GetCascadeIndex() { return _uCascadeIndex; }
	DXGI_FORMAT GetBackBufferRTVFormat() { return _tBackBufferFormat; }
	DXGI_FORMAT GetFloatRTVFormat() { return _tResolvedBufferFormat; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetFloatBufferSrvCpu() { return _hResolvedBufferSrvCpu; }

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
	AkBool CreateRTVsAndSRVsForPBR();
	AkBool CreateDSVs(AkU32 uWidth, AkU32 uHeight);
	AkBool CreateShadowDSVs(AkU32 uWidth, AkU32 uHeight);
	AkBool CreateFence();
	AkBool CreateRenderThreadPool(AkU32 uThreadCount);
	AkBool CreatePostProcess();
	AkBool CreateImGuiInitResource();
	void InitViewports(AkF32 fWidth, AkF32 fHeight);
	void InitScissorRect(AkU32 uWidth, AkU32 uHeight);
	void InitCamera();
	void DestroyDevice();
	void DestroyCmdQueue();
	void DestroyDescriptorForRTV();
	void DestroyDescriptorForDSV();
	void DestroySwapChain();
	void DestroyRTVs();
	void DestroyRTVsAndSRVsForPBR();
	void DestroyDSVs();
	void DestroyShadowDSVs();
	void DestroyFence();
	void DestroyRenderThreadPool(AkU32 uThreadCount);
	void DestroyPostProcess();
	void DestroyImGuiInitResource();

	void Fence();
	void WaitForFenceValue(AkU64 u64ExpectedFenceValue);

	void CalculateMousePickingRayCast(AkF32 fNdcX, AkF32 fNdcY, DirectX::SimpleMath::Ray* pRay, AkF32* pRayLength);

private:
	HWND _hWnd = nullptr;
	AkU32 _uRefCount = 1;
	AkF32 _fDpi = 0.0f;
	ID3D12Device* _pDevice = nullptr;
	ID3D12CommandQueue* _pCmdQueue = nullptr;
	ID3D12DescriptorHeap* _pRTVHeap = nullptr;
	ID3D12DescriptorHeap* _pDSVHeap = nullptr;
	ID3D12DescriptorHeap* _pImGuiHeap = nullptr;
	IDXGISwapChain3* _pSwapChain = nullptr;
	ID3D12Resource* _ppBackBufferRT[SWAP_CHAIN_FRAME_COUNT];
	ID3D12Resource* _pMainDS = nullptr;
	ID3D12Resource* _pShadowDS[CASCADE_SHADOW_MAP_LEVEL] = {};
	ID3D12Fence* _pFence = nullptr;
	D3D12_VIEWPORT _tViewport = {};
	D3D12_VIEWPORT _tShadowViewport = {};
	D3D12_RECT _tScissorRect = {};
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
	Light_t _pSpotLights[MAX_LIGHTS_COUNT] = {};
	AkU32 _uPointLightsNum = 0;
	AkU32 _uSpotLightsNum = 0;
	Vector3 _vLightPos = Vector3(0.0f, 2.5f, 1025.0f);
	D3D12_CPU_DESCRIPTOR_HANDLE _pShadowMapSrvCpu[CASCADE_SHADOW_MAP_LEVEL] = {};
	// For PBR.
	ID3D12Resource* _pResolvedBufferRT = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE _hResolvedBufferSrvCpu = {};
	FPostProcess* _pPostProcess = nullptr;
	DXGI_FORMAT _tBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT _tResolvedBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	AkF32 _fIBLStrength = 1.0f;
};

