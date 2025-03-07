#pragma once

#include "Scene.h"

/*
=============
Scene InGame
=============
*/

class MapObjects;

class SceneInGame : public Scene
{
public:
	~SceneInGame();

	virtual AkBool BeginScene() override;
	virtual AkBool EndScene() override;
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void RenderShadow() override;
	virtual void Render() override;

	AkU32 GetTriangleCount();
	AkU32 GetMapObjectCount() { return BUILDING_BOX_COUNT; }
	AkU32 GetRenderMapObjectCount() { return _uRenderMapObjCount; }

private:
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

	// World Map Container.
	MapObjects* _pWorldMap = nullptr;

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


