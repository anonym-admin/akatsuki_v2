#include "pch.h"
#include "ModelManager.h"
#include "PlayerModel.h"
#include "BRS_74_Model.h"
#include "DancerModel.h"

/*
=================
ModelManager
=================
*/

UModelManager::UModelManager()
{
}

UModelManager::~UModelManager()
{
    CleanUp();
}

AkBool UModelManager::Initialize(UApplication* pApp)
{
    _pApp = pApp;

    return AK_TRUE;
}

AkBool UModelManager::InitDefaultModels()
{
    UModel* pModel = nullptr;

    // Hero.
    pModel = new UPlayerModel;
    if (!pModel->Initialize(_pApp))
    {
        __debugbreak();
        return AK_FALSE;
    }
    _ppModelList[(AkU32)MODEL_TYPE::BLENDER_MODEL_HERO] = pModel;

    // BRS_74.
    pModel = new UBRS_74_Model;
    if (!pModel->Initialize(_pApp))
    {
        __debugbreak();
        return AK_FALSE;
    }
    _ppModelList[(AkU32)MODEL_TYPE::BRS_74] = pModel;

    // Dancer.
    pModel = new UDancerModel;
    if (!pModel->Initialize(_pApp))
    {
        __debugbreak();
        return AK_FALSE;
    }
    _ppModelList[(AkU32)MODEL_TYPE::BLENDER_MODEL_DANCER] = pModel;

    return AK_TRUE;
}

void UModelManager::CleanUpDefaultModels()
{
}

void UModelManager::AddModel()
{
}

void UModelManager::CleanUp()
{
}
