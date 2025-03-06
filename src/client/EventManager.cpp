#include "pch.h"
#include "EventManager.h"
#include "SceneManager.h"
#include "EditorManager.h"
#include "Application.h"

/*
==================
GameEventManager
==================
*/

UGameEventManager::UGameEventManager()
{
}

UGameEventManager::~UGameEventManager()
{
    CleanUp();
}

AkBool UGameEventManager::Initialize(UApplication* pApp)
{
    _pApp = pApp;

    return AK_TRUE;
}

void UGameEventManager::Excute(AkF32 fDeltaTime)
{
    GameEventHandle_t* pEventHandle = nullptr;

    while (pEventHandle = Dispatch())
    {
        switch (pEventHandle->eEventType)
        {
            case GAME_EVENT_TYPE::GAME_EVENT_TYPE_CREATE_GAME_OBJECT:
            {

            }
            break;
            case GAME_EVENT_TYPE::GAME_EVENT_TYPE_SCENE_CHANGE:
            {
                USceneManager* pSceneManager = _pApp->GetSceneManager();
                pSceneManager->ChangeScene(pEventHandle->tSceneChangeParam.eAfter);
            }
            break;
            default:
            {
                __debugbreak();
            }
            break;
        }
    }
}

void UGameEventManager::Reset()
{
    _uWritePos = 0;
    _uReadPos = 0;
}

void UGameEventManager::AddEvent(GameEventHandle_t* pEventHandle)
{
    if (_uWritePos + sizeof(GameEventHandle_t) > MAX_EVENT_HANDLE_LIST_COUNT)
    {
        __debugbreak();
    }

    GameEventHandle_t* pDest = _pEventHandleList + _uWritePos;
    memcpy(pDest, pEventHandle, sizeof(GameEventHandle_t));

    _uWritePos += sizeof(GameEventHandle_t);
}

void UGameEventManager::CleanUp()
{
}

GameEventHandle_t* UGameEventManager::Dispatch()
{
    GameEventHandle_t* pResult = nullptr;

    if (_uReadPos + sizeof(GameEventHandle_t) > _uWritePos)
    {
        return pResult;
    }

    pResult = _pEventHandleList + _uReadPos;

    _uReadPos += sizeof(GameEventHandle_t);

    return pResult;
}

/*
==================
EditorEventManager
==================
*/

UEditorEvenetManager::UEditorEvenetManager()
{
}

UEditorEvenetManager::~UEditorEvenetManager()
{
    CleanUp();
}

AkBool UEditorEvenetManager::Initialize(UApplication* pApp)
{
    _pApp = pApp;

    return AK_TRUE;
}

void UEditorEvenetManager::Excute(AkF32 fDeltaTime)
{
    EditorEventHandle_t* pEventHandle = nullptr;

    while (pEventHandle = Dispatch())
    {
        switch (pEventHandle->eEventType)
        {
        case EDITOR_EVENT_TYPE::EDITOR_EVENT_TYPE_CREATE_GAME_OBJECT:
        {

        }
        break;
        case EDITOR_EVENT_TYPE::EDITOR_EVENT_TYPE_EDITOR_CHANGE:
        {
            UEditorManager* pEditorManager = _pApp->GetEditorManager();
            pEditorManager->ChangeEditor(pEventHandle->tEditorChangeParam.eAfter);
        }
        break;
        default:
        {
            __debugbreak();
        }
        break;
        }
    }
}

void UEditorEvenetManager::Reset()
{
    _uWritePos = 0;
    _uReadPos = 0;
}

void UEditorEvenetManager::AddEvent(EditorEventHandle_t* pEventHandle)
{
    if (_uWritePos + sizeof(EditorEventHandle_t) > MAX_EVENT_HANDLE_LIST_COUNT)
    {
        __debugbreak();
    }

    EditorEventHandle_t* pDest = _pEventHandleList + _uWritePos;
    memcpy(pDest, pEventHandle, sizeof(EditorEventHandle_t));

    _uWritePos += sizeof(EditorEventHandle_t);
}

void UEditorEvenetManager::CleanUp()
{
}

EditorEventHandle_t* UEditorEvenetManager::Dispatch()
{
    EditorEventHandle_t* pResult = nullptr;

    if (_uReadPos + sizeof(EditorEventHandle_t) > _uWritePos)
    {
        return pResult;
    }

    pResult = _pEventHandleList + _uReadPos;

    _uReadPos += sizeof(EditorEventHandle_t);

    return pResult;
}
