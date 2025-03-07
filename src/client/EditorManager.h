#pragma once

/*
=================
EditorManager
=================
*/

class Editor;

class EditorManager
{
public:
	~EditorManager();

	void Update();
	void FinalUpdate();
	void Render();
	void RenderShadow();

	void ChangeEditor(EDITOR_TYPE eEditorType);
	Editor* GetCurrentEditor() { return _pCurEditor; }
	EDITOR_TYPE GetCurrentEditorType() { return _eType; }
	Editor* GetEditor(EDITOR_TYPE eType) { return _pEditorList[(AkU32)eType]; }
	Editor* AddEditor(EDITOR_TYPE eType, Editor* pEditor);
	Editor* BindCurrnetEditor(EDITOR_TYPE eType);
	void UnBindCurrentEditor();

private:
	void CleanUp();

private:
	Editor* _pEditorList[(AkU32)EDITOR_TYPE::EDITOR_TYPE_COUNT] = {};
	Editor* _pCurEditor = nullptr;
	EDITOR_TYPE _eType = {};
	AkU32 _uEditorNum = 0;
};

