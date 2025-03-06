#pragma once

#include "Scene.h"

/*
=============
Scene InGame
=============
*/

class UWorldMapContainer;

class USceneInGame : public UScene
{
public:
	USceneInGame();
	~USceneInGame();

	virtual AkBool Initialize(UApplication* pApp) override;
	virtual AkBool BeginScene() override;
	virtual AkBool EndScene() override;
	virtual void Update(const AkF32 fDeltaTime) override;
	virtual void FinalUpdate(const AkF32 fDeltaTime) override;
	virtual void RenderShadowPass() override;
	virtual void Render() override;

	AkU32 GetTriangleCount();
	AkU32 GetMapObjectCount() { return BUILDING_BOX_COUNT; }
	AkU32 GetRenderMapObjectCount() { return _uRenderMapObjCount; }

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;

	// Mini map
	ISpriteObject* _pMiniMapSprite = nullptr;
	ISpriteObject* _pMiniMapOutlineSprite = nullptr;
	AkI32 _iMiniMapPosX = 0;
	AkI32 _iMiniMapPosY = 0;
	AkI32 _iOffset = 10;

	// Location point
	ISpriteObject* _pLocationPointSprite = nullptr;
	AkI32 _iPosX = 0;
	AkI32 _iPosY = 0;

	// Skybox
	ISkyboxObject* _pSkyboxObj = nullptr;
	Matrix _mSkyboxTransform = Matrix();

	// LandScape
	class ULandScape* _pLandScape = nullptr;

	// World Map Container.
	UWorldMapContainer* _pWorldMap = nullptr;

	// Frustum.
	AkFrustum_t* _pFrustum = nullptr;

public:
	// Map Obj.
	static const AkU32 BUILDING_BOX_COUNT = 2048;
	IMeshObject* _pBuildingMeshObj = nullptr;
	Matrix _pRandomTransform[BUILDING_BOX_COUNT] = {};
	Matrix _pTempRandomTransform[BUILDING_BOX_COUNT] = {};

	AkBox_t* _pBoxList = nullptr;
	AkBool* _pDraw = nullptr;
	AkBool _bFirst = AK_TRUE;
	AkU32 _uRenderMapObjCount = 0;

	// House.
	IMeshObject* _pHouseMeshObj = nullptr;
	Matrix _mHouseWorld = Matrix();
};


