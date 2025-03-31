#include "pch.h"
#include "BillboardModel.h"

/*
===============
BillboardModel
===============
*/

BillboardModels::BillboardModels(BillboardData_t* pBillboardData)
{
    if (!Initialize(pBillboardData))
    {
        __debugbreak();
    }
}

BillboardModels::~BillboardModels()
{
    CleanUp();
}

AkBool BillboardModels::Initialize(BillboardData_t* pBillboardData)
{
    _pBillboard = GRenderer->CreateBillboard();
    _pBillboard->CreateBillboardBuffer(pBillboardData);

    Vector3 vAlbedo = Vector3(1.0f);
    Vector3 vEmissive = Vector3(0.0f);
    _pBillboard->UpdateMaterialBuffers(&vAlbedo, 0.0f, 1.0f, &vEmissive);

    return AK_TRUE;
}

void BillboardModels::RenderDepthMap()
{
    GRenderer->RenderDepthMapOfBillboard(_pBillboard, &_mWorldRow);
}

void BillboardModels::RenderShadowMaps()
{
    GRenderer->RenderShadowOfBillboard(_pBillboard, &_mWorldRow);
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
