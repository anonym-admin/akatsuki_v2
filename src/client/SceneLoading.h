#pragma once

#include "Scene.h"

/*
===============
Loading Scene
===============
*/

class USceneLoading : public UScene
{
public:
	USceneLoading();
	~USceneLoading();

	virtual AkBool Initialize(UApplication* pApp) override;
	virtual AkBool BeginScene() override;
	virtual AkBool EndScene() override;

	void RenderLoadingScreenCallBack(const wchar_t* wcText);

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;

	// Loading scene sprite.
	AkU32 _uScreenWidth = 0;
	AkU32 _uScreenHeight = 0;
	AkU8* _pScreenImage = nullptr;
	void* _pScreenTextureHandle = nullptr;
};

