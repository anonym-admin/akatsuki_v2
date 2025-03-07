#pragma once

#include "Scene.h"

/*
===============
Loading Scene
===============
*/

class SceneLoading : public Scene
{
public:
	~SceneLoading();

	virtual AkBool BeginScene() override;
	virtual AkBool EndScene() override;
	virtual void Update() override {};
	virtual void FinalUpdate() override {};
	virtual void Render() override {};
	virtual void RenderShadow() override {};

	void RenderLoadingScreenCallBack(const wchar_t* wcText);

private:
	// Loading scene sprite.
	AkU32 _uScreenWidth = 0;
	AkU32 _uScreenHeight = 0;
	AkU8* _pScreenImage = nullptr;
	void* _pScreenTextureHandle = nullptr;
};

