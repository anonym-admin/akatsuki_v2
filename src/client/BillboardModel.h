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
	BillboardModels(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum);
	~BillboardModels();

	AkBool Initialize(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum);
	void SetTextureArray(void* pTexHandle);
	virtual void Render() override;

private:
	void CleanUp();

private:
	IBillboard* _pBillboard = nullptr;
};

