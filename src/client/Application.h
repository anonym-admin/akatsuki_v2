#pragma once

/*
===============
Application
===============
*/

class PostRenderControl;
class ModelExporter;
class UIImage;
class UIButton;

class Application
{
public:
	static const AkU32 SYSTEM_FONT_SIZE = 10;
	const wchar_t* SYSTEM_FONT_FAMILY_NAME = L"Consolas";

	Application();
	~Application();

	AkBool InitApplication(AkBool bEnableDebugLayer, AkBool bEnableGBV);
	void RunApplication();
	
	AkBool UpdateWindowSize(AkU32 uScreenWidth, AkU32 uScreenHeight);
	void ProcessInputControl();

private:
	void CleanUp();

	AkBool InitRenderer(AkBool bEnableDebugLayer, AkBool bEnableGBV);
	AkBool InitScene();
	AkBool InitEditor();
	AkBool InitUI();
	AkBool InitTextResource();

	void Update();
	void UpdateEnviroment();
	void UpdateText();
	void Render();
	void RenderText();
	void CalculateFrameRate();
	void ExitGame();

private:
	HMODULE _hRendererDLL = nullptr;

	AkBool _bUseVSync = AK_FALSE;
	AkBool _bChangeEditor = AK_FALSE;
	AkBool _bPlayingEditor = AK_FALSE;

	// Post process control.
	PostRenderControl* _pPostProcess = nullptr;
	AkBool _bUseDebugMode = AK_FALSE;
	AkBool _bUsePostProcessController = AK_FALSE;

	AkU32 _uTextTextureWidth = 0;
	AkU32 _uTextTextureHeight = 0;
	AkU8* _pTextTextureImage = nullptr;
	void* _pTextTextureHandle = nullptr;
	wchar_t _wcText[256] = {};

	AkU32 _uScreenTextureWidth = 0;
	AkU32 _uScreenTextureHeight = 0;
	AkU8* _pScreenTextureImage = nullptr;
	void* _pScreenTextureHandle = nullptr;
	void* _pScreenTextFontObj = nullptr;

	AkI32 _iEditorType = -1;
};


