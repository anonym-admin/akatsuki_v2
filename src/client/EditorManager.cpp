#include "pch.h"
#include "EditorManager.h"
#include "EditorMap.h"
#include "EditorModel.h"

UEditorManager::UEditorManager()
{
}

UEditorManager::~UEditorManager()
{
	CleanUp();
}

AkBool UEditorManager::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	{
		UEditor* pEditor = CreateEditor(EDITOR_TYPE::EDITOR_TYPE_CHARACTER);
	}

	{
		UEditor* pEditor = CreateEditor(EDITOR_TYPE::EDITOR_TYPE_MAP);
	}

	_pCurEditor = _pEditorList[(AkU32)EDITOR_TYPE::EDITOR_TYPE_CHARACTER];

	return AK_TRUE;
}

void UEditorManager::Update(const AkF32 fDeltaTime)
{
	if (_pCurEditor)
	{
		_pCurEditor->Update(fDeltaTime);
	}
}

void UEditorManager::Render()
{
	if (_pCurEditor)
	{
		_pCurEditor->Render();
	}
}

void UEditorManager::ChangeEditor(EDITOR_TYPE eEditorType)
{
	if (!_pCurEditor)
	{
		__debugbreak();
		return;
	}

	_pCurEditor->EndEditor();

	_pCurEditor = _pEditorList[(AkU32)eEditorType];

	_pCurEditor->BeginEditor();
}

void UEditorManager::CleanUp()
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

UEditor* UEditorManager::CreateEditor(EDITOR_TYPE eEditorType)
{
	UEditor* pEditor = nullptr;

	switch (eEditorType)
	{
	case EDITOR_TYPE::EDITOR_TYPE_CHARACTER:
		pEditor = new UEditorModel;
		break;
	case EDITOR_TYPE::EDITOR_TYPE_MAP:
		pEditor = new UEditorMap;
		break;
	}

	pEditor->Initialize(_pApp);
	_pEditorList[(AkU32)eEditorType] = pEditor;
	_uEditorNum++;

	return pEditor;
}
