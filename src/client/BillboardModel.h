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

private:
	void CleanUp();

private:
	IBillboard* _pBillboard = nullptr;

	void* _pTreeTextureArray = nullptr;
};

