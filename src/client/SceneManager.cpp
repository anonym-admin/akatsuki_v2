#include "pch.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneInGame.h"

/*
===================
USceneManager
===================
*/

USceneManager::USceneManager()
{
}

USceneManager::~USceneManager()
{
    CleanUp();
}

AkBool USceneManager::Initialize(UApplication* pApp)
{
    _pApp = pApp;
    
    {
        UScene* pScene = CreateScene(GAME_SCENE_TYPE::SCENE_TYPE_LOADING);
    }

    {
        UScene* pScene = CreateScene(GAME_SCENE_TYPE::SCENE_TYPE_INGANE);
    }

    _pCurScene = _pSceneList[(AkU32)GAME_SCENE_TYPE::SCENE_TYPE_LOADING];

    _pCurScene->BeginScene();

    return AK_TRUE;
}

void USceneManager::Update(const AkF32 fDeltaTime)
{
    if (_pCurScene)
    {
        _pCurScene->Update(fDeltaTime);
    }
}

void USceneManager::FinalUpdate(const AkF32 fDeltaTime)
{
    if (_pCurScene)
    {
        _pCurScene->FinalUpdate(fDeltaTime);
    }
}

void USceneManager::RenderShadowPass()
{
    if (_pCurScene)
    {
        _pCurScene->RenderShadowPass();
    }
}

void USceneManager::Render()
{
    if (_pCurScene)
    {
        _pCurScene->Render();
    }
}

void USceneManager::ChangeScene(GAME_SCENE_TYPE eSceneType)
{
    if (!_pCurScene)
    {
        __debugbreak();
        return;
    }

    _pCurScene->EndScene();

    _pCurScene = _pSceneList[(AkU32)eSceneType];

    _pCurScene->BeginScene();
}

void USceneManager::CleanUp()
{
    for (AkU32 i = 0; i < _uSceneNum; i++)
    {
        if (_pSceneList[i])
        {
            delete _pSceneList[i];
            _pSceneList[i] = nullptr;
        }
    }
}

UScene* USceneManager::CreateScene(GAME_SCENE_TYPE eSceneType)
{
    UScene* pScene = nullptr;

    switch (eSceneType)
    {
    case GAME_SCENE_TYPE::SCENE_TYPE_LOADING:
        pScene = new USceneLoading;
        break;
    case GAME_SCENE_TYPE::SCENE_TYPE_INGANE:
        pScene = new USceneInGame;
        break;
    }

    pScene->Initialize(_pApp);
    _pSceneList[(AkU32)eSceneType] = pScene;
    _uSceneNum++;

    return pScene;
}

