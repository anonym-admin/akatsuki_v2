#pragma once

/*
==============
UI Manager
==============
*/

class Application;
class UUI;
class TextUI;

class UIManager
{
public:
	~UIManager();

	void Update();
	void Render();
	
	void AddUI(UUI* pUI, UI_TYPE eType);
	void OnUI(UI_TYPE eType);
	void OffUI(UI_TYPE eType);
	void ToggleUI(UI_TYPE eType);

private:
	void CleanUp();

	void ProcessMouseControl();
	UUI* GetTargetedUI(UUI* pRootUI);

private:
	UUI* _ppUIList[(AkU32)UI_TYPE::UI_OBJ_TYPE_COUNT] = {};
	AkBool _pOnFlag[(AkU32)UI_TYPE::UI_OBJ_TYPE_COUNT] = {};
};

