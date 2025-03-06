#pragma once

class UApplication;
class UEditor;

class UEditorManager
{
public:
	UEditorManager();
	~UEditorManager();

	AkBool Initialize(UApplication* pApp);
	void Update(const AkF32 fDeltaTime);
	void Render();

	void ChangeEditor(EDITOR_TYPE eEditorType);
	UEditor* GetCurrentEditor() { return _pCurEditor; }
	UEditor* GetEditor(EDITOR_TYPE eEditorType) { return _pEditorList[(AkU32)eEditorType]; }

private:
	void CleanUp();

	UEditor* CreateEditor(EDITOR_TYPE eEditorType);

private:
	UApplication* _pApp = nullptr;
	UEditor* _pEditorList[(AkU32)EDITOR_TYPE::EDITOR_TYPE_COUNT] = {};
	UEditor* _pCurEditor = nullptr;
	AkU32 _uEditorNum = 0;
};

