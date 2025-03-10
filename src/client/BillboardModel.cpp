#include "pch.h"
#include "BillboardModel.h"

/*
===============
BillboardModel
===============
*/

BillboardModels::BillboardModels(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum)
{
    if (!Initialize(pBillboardVertices, uPointNum))
    {
        __debugbreak();
    }
}

BillboardModels::~BillboardModels()
{
}

AkBool BillboardModels::Initialize(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum)
{
    _pBillboard = GRenderer->CreateBillboards();
    _pBillboard->CreateBillboardBuffer(pBillboardVertices, uPointNum);
    return AK_TRUE;
}

void BillboardModels::SetTextureArray(void* pTexHandle)
{
    _pBillboard->SetTextureArray(pTexHandle);
}

void BillboardModels::Render()
{
    GRenderer->RenderBillboard(_pBillboard, &_mWorldRow);
}

void BillboardModels::CleanUp()
{
    if (_pBillboard)
    {
        _pBillboard->Release();
        _pBillboard = nullptr;
    }
}
