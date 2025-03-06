#include "pch.h"
#include "UIManager.h"
#include "UI.h"
#include "TextUI.h"
#include "GameInput.h"
#include "Application.h"

/*
==============
UI Manager
==============
*/

UUIManager::UUIManager()
{
}

UUIManager::~UUIManager()
{
    CleanUp();
}

AkBool UUIManager::Initialize(UApplication* pApp)
{
    _pApp = pApp;

    return AK_TRUE;
}

void UUIManager::Update(const AkF32 fDeltaTime)
{
    for (AkU32 i = 0; i < (AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT; i++)
    {
        if(_pOnFlag[i])
            _ppUIList[i]->Update(fDeltaTime);
    }

    ProcessMouseControl();
}

void UUIManager::Render()
{
    for (AkU32 i = 0; i < (AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT; i++)
    {
        if (_pOnFlag[i])
            _ppUIList[i]->Render();
    }
}

void UUIManager::AddUI(UUI* pUI, UI_OBJECT_TYPE eType)
{
    _ppUIList[(AkU32)eType] = pUI;
}

void UUIManager::OnUI(UI_OBJECT_TYPE eType)
{
    _pOnFlag[(AkU32)eType] = AK_TRUE;
}

void UUIManager::OffUI(UI_OBJECT_TYPE eType)
{
    _pOnFlag[(AkU32)eType] = AK_FALSE;
}

void UUIManager::ToggleUI(UI_OBJECT_TYPE eType)
{
    _pOnFlag[(AkU32)eType] = !_pOnFlag[(AkU32)eType];
}

void UUIManager::CleanUp()
{
    for (AkU32 i = 0; i < (AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT; i++)
    {
        delete _ppUIList[i];
        _ppUIList[i] = nullptr;
    }
}

void UUIManager::ProcessMouseControl()
{
    UGameInput* pGameInput = _pApp->GetGameInput();

    for (AkU32 i = 0; i < (AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT; i++)
    {
        UUI* pUI = (UUI*)_ppUIList[i];

        if (nullptr == pUI)
        {
            continue;
        }

        pUI = GetTargetedUI(pUI);

        if (nullptr != pUI)
        {
            if (pUI->IsMouseOn())
            {
                pUI->MouseOn();

                if (pGameInput->LeftBtnDown())
                {
                    pUI->MouseLBtnDown();
                }
                else if (pGameInput->LeftBtnUp())
                {
                    if (pUI->IsMouseLBtnDown())
                    {
                        pUI->MouseLBtnClick();
                    }

                    pUI->MouseLBtnUp();
                }
            }
            else
            {
                // TODO!!
                if (pUI->IsMouseLBtnDown())
                {
                    pUI->MouseLBtnUp();
                }
            }
        }
    }
}

UUI* UUIManager::GetTargetedUI(UUI* pRootUI)
{
    Queue_t* pQueue = nullptr;

    CQ_CreateQueue(&pQueue, 10, sizeof(UUI*));

    CQ_Enqueue(pQueue, pRootUI);

    UUI* pTargetedUI = nullptr;
    while (!CQ_IsEmpty(pQueue))
    {
        UUI* pUI = (UUI*)CQ_Dequeue(pQueue);

        if (pUI->IsMouseOn())
        {
            pTargetedUI = pUI;
        }
        else
        {
            // TODO!!
            if (pUI->IsMouseLBtnDown())
            {
                pUI->MouseLBtnUp();
            }
        }

        List_t* pChild = pUI->GetChildUIListHead();
        while (pChild != nullptr)
        {
            CQ_Enqueue(pQueue, pChild->pData);

            pChild = pChild->pNext;
        }
    }

    CQ_DestroyQueue(pQueue);

    return pTargetedUI;
}
