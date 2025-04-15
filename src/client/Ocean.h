#pragma once

#include "ModelObject.h"

class Ocean : public ModelObject
{
public:
	Ocean();
	~Ocean();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;

private:
	void CleanUp();

private:
	IEnvironmentObject* _pOceanObj = nullptr;
};

