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
    CleanUp();
}

AkBool BillboardModels::Initialize(BillboardVertex_t* pBillboardVertices, AkU32 uPointNum)
{
    _pBillboard = GRenderer->CreateBillboards();
    _pBillboard->CreateBillboardBuffer(pBillboardVertices, uPointNum);

    _pTreeTextureArray = GRenderer->CreateTextureFromFile(L"../../assets/tree02S.dds", AK_TRUE);

    return AK_TRUE;
}

void BillboardModels::Render()
{
    GRenderer->RenderBillboard(_pBillboard, &_mWorldRow, _pTreeTextureArray);
}

void BillboardModels::CleanUp()
{
    if (_pTreeTextureArray)
    {
        GRenderer->DestroyTexture(_pTreeTextureArray);
        _pTreeTextureArray = nullptr;
    }
    if (_pBillboard)
    {
        _pBillboard->Release();
        _pBillboard = nullptr;
    }
}
