#include "pch.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneInGame.h"

/*
===================
USceneManager
===================
*/


SceneManager::~SceneManager()
{
    CleanUp();
}

void SceneManager::Update()
{
    if (_pCurScene)
    {
        _pCurScene->Update();
    }
}

void SceneManager::FinalUpdate()
{
    if (_pCurScene)
    {
        _pCurScene->FinalUpdate();
    }
}

void SceneManager::RenderShadow()
{
    if (_pCurScene)
    {
        _pCurScene->RenderShadow();
    }
}

void SceneManager::Render()
{
    if (_pCurScene)
    {
        _pCurScene->Render();
    }
}

void SceneManager::ChangeScene(SCENE_TYPE eSceneType)
{
    if (!_pCurScene)
    {
        __debugbreak();
        return;
    }

    _pCurScene->EndScene();

    _pCurScene = _pSceneList[(AkU32)eSceneType];

    _pCurScene->BeginScene();

    _eType = eSceneType;
}

Scene* SceneManager::AddScene(SCENE_TYPE eType, Scene* pScene)
{
    if (nullptr != _pSceneList[(AkU32)eType])
        return _pSceneList[(AkU32)eType];

    _pSceneList[(AkU32)eType] = pScene;
    _uSceneNum++;
    return  _pSceneList[(AkU32)eType];
}

Scene* SceneManager::BindCurrentScene(SCENE_TYPE eType)
{
    _pCurScene = _pSceneList[(AkU32)eType];
    _eType = eType;
    return _pCurScene;
}

void SceneManager::UnBindCurrentScene()
{
    _pCurScene = nullptr;
}

void SceneManager::CleanUp()
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