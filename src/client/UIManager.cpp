#include "pch.h"
#include "UIManager.h"
#include "UI.h"
#include "TextUI.h"
#include "GameInput.h"

/*
==============
UI Manager
==============
*/

UIManager::~UIManager()
{
    CleanUp();
}

void UIManager::Update()
{
    for (AkU32 i = 0; i < (AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT; i++)
    {
        if(_pOnFlag[i])
            _ppUIList[i]->Update();
    }

    ProcessMouseControl();
}

void UIManager::Render()
{
    for (AkU32 i = 0; i < (AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT; i++)
    {
        if (_pOnFlag[i])
            _ppUIList[i]->Render();
    }
}

void UIManager::AddUI(UUI* pUI, UI_OBJECT_TYPE eType)
{
    _ppUIList[(AkU32)eType] = pUI;
}

void UIManager::OnUI(UI_OBJECT_TYPE eType)
{
    _pOnFlag[(AkU32)eType] = AK_TRUE;
}

void UIManager::OffUI(UI_OBJECT_TYPE eType)
{
    _pOnFlag[(AkU32)eType] = AK_FALSE;
}

void UIManager::ToggleUI(UI_OBJECT_TYPE eType)
{
    _pOnFlag[(AkU32)eType] = !_pOnFlag[(AkU32)eType];
}

void UIManager::CleanUp()
{
    for (AkU32 i = 0; i < (AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT; i++)
    {
        delete _ppUIList[i];
        _ppUIList[i] = nullptr;
    }
}

void UIManager::ProcessMouseControl()
{
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

                if (LBTN_DOWN)
                {
                    pUI->MouseLBtnDown();
                }
                else if (LBTN_UP)
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

UUI* UIManager::GetTargetedUI(UUI* pRootUI)
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
