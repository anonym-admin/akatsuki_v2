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
	virtual void Load(const wchar_t* wcSceneFile) override;

private:
	// Mini map
	ISprite* _pMiniMapSprite = nullptr;
	ISprite* _pMiniMapOutlineSprite = nullptr;
	AkI32 _iMiniMapPosX = 0;
	AkI32 _iMiniMapPosY = 0;
	AkI32 _iOffset = 10;

	// Location point
	ISprite* _pLocationPointSprite = nullptr;
	AkI32 _iPosX = 0;
	AkI32 _iPosY = 0;

	// Skybox
	ISkybox* _pSkyboxObj = nullptr;
	Matrix _mSkyboxTransform = Matrix();
};


