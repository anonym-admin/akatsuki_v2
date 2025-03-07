#include "pch.h"
#include "EventManager.h"
#include "SceneManager.h"
#include "Application.h"

/*
==================
GameEventManager
==================
*/

EventManager::~EventManager()
{
    CleanUp();
}

void EventManager::Excute()
{
    EventHandle_t* pEventHandle = nullptr;

    while (pEventHandle = Dispatch())
    {
        switch (pEventHandle->eEventType)
        {
            case EVENT_TYPE::CREATE_GAME_OBJECT:
            {

            }
            break;
            case EVENT_TYPE::SCENE_CHANGE:
            {
                GSceneManager->ChangeScene(pEventHandle->tSceneChangeParam.eAfter);
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

void EventManager::Reset()
{
    _uWritePos = 0;
    _uReadPos = 0;
}

void EventManager::AddEvent(EventHandle_t* pEventHandle)
{
    if (_uWritePos + sizeof(EventHandle_t) > MAX_EVENT_HANDLE_LIST_COUNT)
    {
        __debugbreak();
    }

    EventHandle_t* pDest = _pEventHandleList + _uWritePos;
    memcpy(pDest, pEventHandle, sizeof(EventHandle_t));

    _uWritePos += sizeof(EventHandle_t);
}

void EventManager::CleanUp()
{
}

EventHandle_t* EventManager::Dispatch()
{
    EventHandle_t* pResult = nullptr;

    if (_uReadPos + sizeof(EventHandle_t) > _uWritePos)
    {
        return pResult;
    }

    pResult = _pEventHandleList + _uReadPos;

    _uReadPos += sizeof(EventHandle_t);

    return pResult;
}
