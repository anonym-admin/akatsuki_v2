#pragma once

/*
==============
UI Manager
==============
*/

class UApplication;
class UUI;
class UTextUI;

class UUIManager
{
public:
	UUIManager();
	~UUIManager();

	AkBool Initialize(UApplication* pApp);
	void Update(const AkF32 fDeltaTime);
	void Render();
	
	void AddUI(UUI* pUI, UI_OBJECT_TYPE eType);
	void OnUI(UI_OBJECT_TYPE eType);
	void OffUI(UI_OBJECT_TYPE eType);
	void ToggleUI(UI_OBJECT_TYPE eType);

private:
	void CleanUp();

	void ProcessMouseControl();
	UUI* GetTargetedUI(UUI* pRootUI);

private:
	UApplication* _pApp = nullptr;

	UUI* _ppUIList[(AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT] = {};
	AkBool _pOnFlag[(AkU32)UI_OBJECT_TYPE::UI_OBJ_TYPE_COUNT] = {};
};

