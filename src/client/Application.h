#pragma once

/*
* U : User defined class
* A : Engine defined class
* F : Module defined class
* I : Interface class
*/

/*
===============
Application
===============
*/


class TextUI;
class InputUI;

class Application
{
public:
	Application();
	~Application();

	AkBool InitApplication(AkBool bEnableDebugLayer, AkBool bEnableGBV);
	void RunApplication();

	AkBool UpdateWindowSize(AkU32 uScreenWidth, AkU32 uScreenHeight);
	
	AkBool EnableEditor() { return _bEnableEditor; }
	AkF32 GetClampNdcX() { return _fClampNdcX; }
	AkF32 GetClampNdcY() { return _fClampNdcY; }
	void SetVSync(AkBool bUseVSync);

private:
	void CleanUp();

	AkBool InitRenderer(AkBool bEnableDebugLayer, AkBool bEnableGBV);
	AkBool InitScene();
	AkBool InitUI();

	void Update();
	void UpdateEnviroment();
	void UpdateText();
	void Render();
	void CalculateFrameRate();
	void ExitGame();

private:
	HMODULE _hRendererDLL = nullptr;

	AkF32 _fFps = 0.0f;
	AkF32 _fClampNdcX = 0.0f;
	AkF32 _fClampNdcY = 0.0f;

	AkU32 _uEditorType = 0;
	AkBool _bUpdatedFirst = AK_TRUE;
	AkBool _bEnableEditor = AK_FALSE;

	TextUI* _pSysTextUI = nullptr;
	InputUI* _pDynamicTextUI = nullptr;
	Vector3 _vSysFontColor = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 _vDynamicTextFontColor = Vector3(1.0f);
	AkBool _bUseVSync = AK_FALSE;

	AkBool _bESC = AK_FALSE;

public:
	AkF32 _fNdcX = 0.0f;
	AkF32 _fNdcY = 0.0f;
};


