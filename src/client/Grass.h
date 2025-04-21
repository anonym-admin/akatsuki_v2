#pragma once

#include "ModelObject.h"

class GrassModel;

class Grass : public ModelObject
{
public:
	Grass();
	Grass(const Grass& Other);
	~Grass();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderDepthMap() override {};
	virtual void RenderShadowMaps() override {};

	virtual void RenderGUI() override;

	virtual Grass* Clone() override;

private:
	void CleanUp();

private:
	GrassModel* _pGrassModel = nullptr;
};

