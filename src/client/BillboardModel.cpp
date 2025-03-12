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

BillboardModels::BillboardModels(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
    if (!Initialize(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive))
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

AkBool BillboardModels::Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
    _pBillboard = GRenderer->CreateBillboards();
    _pBillboard->CreateMeshBuffers(pMeshData, uMeshDataNum);
    _pBillboard->UpdateMaterialBuffers(pAlbedo, fMetallic, fRoughness, pEmissive);

    return AK_TRUE;
}

void BillboardModels::Render()
{
    if(_pTreeTextureArray)
    {
        GRenderer->RenderBillboardWithGS(_pBillboard, &_mWorldRow, _pTreeTextureArray);
    }
    else
    {
        GRenderer->RenderBillboard(_pBillboard, &_mWorldRow);
    }
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
