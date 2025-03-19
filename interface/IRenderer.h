#pragma once

#include <combaseapi.h>
#include "common/CommonMeshData.h"
#include "common/CommonEnum.h"
#include "common/Math.h"

interface IMeshObject : public IUnknown
{
	virtual AkBool CreateMeshBuffers(MeshData_t * pMeshData, AkU32 uMeshDataNum) = 0;
	virtual AkBool UpdateMaterialBuffers(const Vector3* pAlbedoFactor, AkF32 fMetallicFactor, AkF32 fRoughnessFactor, const Vector3* pEmisiionFactor) = 0;
	virtual void EnableWireFrame() = 0;
	virtual void DisableWireFrame() = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

interface ISprite : public IUnknown
{
	virtual void SetDrawBackground(AkBool bDrawBackground) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

interface ISkybox : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR * __RPC_FAR * ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

interface ILineObject : public IUnknown
{
	virtual AkBool CreateLineBuffer(VertexColor_t * pStart, VertexColor_t * pEnd) = 0;
	virtual AkBool CreateLineBuffers(LineData_t* pLineData) = 0;
	virtual void SetColor(AkF32 fR, AkF32 fG, AkF32 fB) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

interface IBillboard : public IUnknown
{
	virtual AkBool CreateBillboardBuffer(BillboardData_t * pBillboardData) = 0;
	virtual AkBool UpdateMaterialBuffers(const Vector3* pAlbedoFactor, AkF32 fMetallicFactor, AkF32 fRoughnessFactor, const Vector3* pEmisiionFactor) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

interface ITerrain : public IUnknown
{
	virtual AkBool CreateStaticMeshBuffers(VertexNormalTexcoordTangentAlpha_t * pVertices, AkU32 uVerticeNum, AkU32 * pIndices, AkU32 uIndiceNum) = 0;
	virtual void* CreateDynamicMeshBuffers(VertexNormalTexcoordTangentAlpha_t* pVertices, AkU32 uVerticeNum, AkU32* pIndices, AkU32 uIndiceNum) = 0;
	virtual void SetTextures(const wchar_t* wcSecondFilename, const wchar_t* wcThirdFilename, const wchar_t* wcAlbedoFilename = nullptr, const wchar_t* wcNormalFilename = nullptr, const wchar_t* wcEmissvieFilename = nullptr, const wchar_t* wcMetallicFilename = nullptr, const wchar_t* wcRoughnessFilename = nullptr, const wchar_t* wcAOFilename = nullptr) = 0;
	virtual AkBool UpdateMaterialBuffers(const Vector3* pAlbedoFactor, AkF32 fMetallicFactor, AkF32 fRoughnessFactor, const Vector3* pEmisiionFactor) = 0;
	virtual void EnableWireFrame() = 0;
	virtual void DisableWireFrame() = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

interface IParticle : public IUnknown
{
	virtual void* CreateParticleSpark(VertexParticle_t* pVertices, AkU32 uNum) = 0;
	virtual void* CreateParticleSprite(VertexSize_t* pVerices) = 0;
	virtual void SetTexture(void* pTexHandle) = 0;
	virtual void DestroyBasicParticleBuffer(void* pDBHandle) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};

interface IRenderer : public IUnknown
{
	virtual AkBool Initialize(HWND hWnd, AkBool bEnableDebugLayer, AkBool bEnableGBV) = 0;
	virtual void BeginRender() = 0;
	virtual void EndRender() = 0;
	virtual void BeginCasterRenderPreparation() = 0;
	virtual void EndCasterRenderPreparation() = 0;
	virtual void Present() = 0;
	virtual IMeshObject* CreateBasicMeshObject() = 0;
	virtual IMeshObject* CreateSkinnedMeshObject() = 0;
	virtual ISprite* CreateSpriteObject() = 0;
	virtual ISprite* CreateSpriteObjectWidthTex(const wchar_t* wcTexFilename, AkI32 iPosX, AkI32 iPosY, AkI32 iWidth, AkI32 iHeight) = 0;
	virtual ISkybox* CreateSkyboxObject() = 0;
	virtual ILineObject* CreateLineObject() = 0;
	virtual IBillboard* CreateBillboard() = 0;
	virtual ITerrain* CreateTerrain() = 0;
	virtual IParticle* CreateParticle() = 0;
	virtual void* CreateTextureFromFile(const wchar_t* wcFilename, AkBool bUseSRGB, AkBool bIsArray = AK_FALSE) = 0;
	virtual void* CreateCubeMapTexture(const wchar_t* wcFilename) = 0;
	virtual void* CreateDynamicTexture(AkU32 uTexWidth, AkU32 uTexHeight) = 0;
	virtual void* CreateFontObject(const wchar_t* wcFontFamilyName, AkF32 fFontSize) = 0;
	virtual void BindIBLTexture(void* pIrradianceTexHandle, void* pSpecularTexHandle, void* pBrdfTexHandle) = 0;
	virtual void BindImGui(void** ppImGuiCtx) = 0;
	virtual void UnBindImGui() = 0;
	virtual AkBool WriteTextToBitmap(AkU8* pDestImage, AkU32 uDestWidth, AkU32 uDestHeight, AkU32 uDestPitch, AkI32* pWidth, AkI32* pHeight, void* pFontHandle, const wchar_t* wcText, AkU32 uTextLength, FONT_COLOR_TYPE eFontColor = FONT_COLOR_TYPE::FONT_COLOR_TYPE_WHITE) = 0;
	virtual AkBool UpdateWindowSize(AkU32 uScreenWidth, AkU32 uScreenHeight) = 0;
	virtual void UpdateTextureWidthImage(void* pTexHandle, const AkU8* pSrcImage, AkU32 uSrcWidth, AkU32 uSrcHeight) = 0;
	virtual void UpdateCascadeOrthoProjMatrix() = 0;
	virtual void UpdateDynamicDefaultBuffer(void* pDBHandle, const void* pData) = 0;
	virtual void UpdateDynamicVertexBuffer(void* pDVHandle, const void* pData) = 0;
	virtual void DestroyTexture(void* pTexHandle) = 0;
	virtual void DestroyFontObject(void* pFontHandle) = 0;
	virtual void DestroyDynamicVertex(void* pDVHandle) = 0;
	virtual void RenderBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) = 0;
	virtual void RenderNormalOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) = 0;
	virtual void RenderShadowOfBasicMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat) = 0;
	virtual void RenderSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) = 0;
	virtual void RenderNormalOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) = 0;
	virtual void RenderShadowOfSkinnedMeshObject(IMeshObject* pMeshObj, const Matrix* pWorldMat, const Matrix* pBonesTransform) = 0;
	virtual void RenderSpriteWithTex(void* pSpriteObjHandle, AkI32 iPosX, AkI32 iPosY, AkF32 fScaleX, AkF32 fScaleY, const RECT* pRect, AkF32 fZ, void* pTexHandle, AkBool bUseBlend) = 0;
	virtual void RenderSprite(void* pSpriteObjHandle, AkI32 iPosX, AkI32 iPosY, AkF32 fScaleX, AkF32 fScaleY, AkF32 fZ, AkBool bUseBlend) = 0;
	virtual void RenderSkybox(ISkybox* pSkyboxObj, const Matrix* pWorldMat, void* pEnvHDR, void* pDiffuseHDR, void* pSpecularHDR) = 0;
	virtual void RenderLineObject(ILineObject* pLineObj, const Matrix* pWorldMat) = 0;
	virtual void RenderBillboard(IBillboard* pBillboard, const Matrix* pWorldMat) = 0;
	virtual void RenderTerrain(ITerrain* pTerrain, const Matrix* pWorldMat, void* pBrush) = 0;
	virtual void RenderNormalOfTerrain(ITerrain* pTerrain, const Matrix* pWorldMat, void* pBrush) = 0;
	virtual void RenderParticleSpark(IParticle* pParticle, const Matrix* pWorldRow, void* pDBHandle, AkU32 uParticleNum, AkF32 fTime, AkF32 fDuration, const Vector2* pStartSize, const Vector3* pStartDirection, AkF32 fSizeOverLifeTime, const Vector3* pRotOverLifeTime, const Vector4* pTotalColor, const Vector4* pColorOverLifeTime) = 0;
	virtual void RenderParticleSprite(IParticle* pParticle, void* pDBHandle, const Vector2* pMaxFrame, const Vector2* pCurFrame) = 0;
	virtual void SetCameraPosition(AkF32 fX, AkF32 fY, AkF32 fZ) = 0;
	virtual void RotateXCamera(AkF32 fRadian) = 0;
	virtual void RotateYCamera(AkF32 fRadian) = 0;
	virtual void RotateYawPitchRollCamera(AkF32 fYaw, AkF32 fPitch, AkF32 fRoll) = 0;
	virtual void MoveCamera(AkF32 fX, AkF32 fY, AkF32 fZ) = 0;
	virtual void GetCameraPosition(AkF32* pX, AkF32* pY, AkF32* pZ) = 0;
	virtual void SetCamera(const Vector3* pCamPos, const Vector3* pCamDir, Vector3* pCamUp) = 0;
	virtual void AddGlobalLight(const Vector3* pRadiance, const Vector3* pDir, AkBool bShadow) = 0;
	virtual void AddPointLight(const Vector3* pPos, const Vector3* pDir, AkF32 fRadius, AkF32 fFallOffStart, AkF32 fFallOffEnd, AkF32 fSpotPower, AkBool bShadow) = 0;
	virtual void SetIBLStrength(AkF32 fIBLStrength) = 0;
	virtual void SetToneMappingType(AkI32 iType) = 0;
	virtual void SetBloomLevels(AkU32 uLevel) = 0;
	virtual void SetBloomStrength(AkF32 uStrength) = 0;
	virtual Vector3 GetWorldNearPosition(AkF32 fNdcX, AkF32 fNdcY) = 0;
	virtual Vector3 GetWorldFarPosition(AkF32 fNdcX, AkF32 fNdcY) = 0;
	virtual void GetViewPorjMatrix(Matrix* pViewMat, Matrix* pProjMat) = 0;
	virtual AkBool MousePickingToPlane(DirectX::SimpleMath::Plane* pPlane, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) = 0;
	virtual AkBool MousePickingToTriangle(Vector3* pV0, Vector3* pV1, Vector3* pV2, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) = 0;
	virtual AkBool MousePickingToSqaure(Vector3* pV0, Vector3* pV1, Vector3* pV2, Vector3* pV3, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) = 0;
	virtual AkBool MousePickingToSphere(DirectX::BoundingSphere* pSphere, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) = 0;
	virtual AkBool MousePickingToBox(DirectX::BoundingBox* pBox, AkF32 fNdcX, AkF32 fNdcY, Vector3* pHitPos, AkF32* pHitDist, AkF32* pRatio) = 0;
	virtual void GetFrustum(Vector4* ppOutPlane) = 0;
	virtual void SetVSync(AkBool bUseVSync) = 0;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
	virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};