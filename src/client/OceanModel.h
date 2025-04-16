#pragma once

#include "Model.h"

class OceanModel : public Model
{
public:
	OceanModel();
	~OceanModel();

	AkBool Initialize();
	virtual void Render() override;

private:
	void CleanUp();

private:
	IEnvironmentObject* _pOceanObj = nullptr;
};

