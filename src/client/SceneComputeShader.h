#pragma once

#include "Scene.h"

class SceneComputeShader : public Scene
{
public:
	~SceneComputeShader();

	virtual AkBool BeginScene() override;
	virtual AkBool EndScene() override;
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;
};

