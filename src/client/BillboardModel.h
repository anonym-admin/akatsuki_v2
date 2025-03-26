#pragma once

#include "Model.h"

/*
===============
BillboardModel
===============
*/

class BillboardModels : public Model
{
public:
	BillboardModels(BillboardData_t* pBillboardData);
	~BillboardModels();

	AkBool Initialize(BillboardData_t* pBillboardData);
	virtual void Render() override;
	virtual void RenderShadow() override;

private:
	void CleanUp();

private:
	IBillboard* _pBillboard = nullptr;
};

