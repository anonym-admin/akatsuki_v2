#include "pch.h"
#include "EditorManager.h"
#include "Editor.h"

/*
=================
EditorManager
=================
*/

EditorManager::~EditorManager()
{
    CleanUp();
}

void EditorManager::Update()
{
    if (_pCurEditor)
    {
        _pCurEditor->Update();
    }
}

void EditorManager::FinalUpdate()
{
    if (_pCurEditor)
    {
        _pCurEditor->FinalUpdate();
    }
}

void EditorManager::RenderShadow()
{
    if (_pCurEditor)
    {
        _pCurEditor->RenderShadow();
    }
}

void EditorManager::Render()
{
    if (_pCurEditor)
    {
        _pCurEditor->Render();
    }
}

void EditorManager::ChangeEditor(EDITOR_TYPE eEditorType)
{
    if (!_pCurEditor)
    {
        __debugbreak();
        return;
    }

    _pCurEditor->EndEditor();

    _pCurEditor = _pEditorList[(AkU32)eEditorType];

    _pCurEditor->BeginEditor();

    _eType = eEditorType;
}

Editor* EditorManager::AddEditor(EDITOR_TYPE eType, Editor* pScene)
{
    if (nullptr != _pEditorList[(AkU32)eType])
        return _pEditorList[(AkU32)eType];

    _pEditorList[(AkU32)eType] = pScene;
    _uEditorNum++;
    return  _pEditorList[(AkU32)eType];
}

Editor* EditorManager::BindCurrnetEditor(EDITOR_TYPE eType)
{
    _pCurEditor = _pEditorList[(AkU32)eType];
    _eType = eType;
    return _pCurEditor;
}

void EditorManager::UnBindCurrentEditor()
{
    _pCurEditor = nullptr;
}

void EditorManager::CleanUp()
{
    for (AkU32 i = 0; i < _uEditorNum; i++)
    {
        if (_pEditorList[i])
        {
            delete _pEditorList[i];
            _pEditorList[i] = nullptr;
        }
    }
}



