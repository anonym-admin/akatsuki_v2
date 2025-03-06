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

class UTimer;
class UGameInput;
class UBaseObject;
class UCollisionManager;
class USceneManager;
class UEditorManager;
class UAssetManager;
class UAnimator;
class UGameEventManager;
class UEditorEvenetManager;
class UModelManager;
class UUIManager;
class UTextUI;
class UInputUI;

class USoundManager;
class USound;

class UApplication
{
public:
	UApplication();
	~UApplication();

	AkBool InitApplication(HWND hWnd, AkBool bEnableDebugLayer, AkBool bEnableGBV);
	void RunApplication();

	AkBool UpdateWindowSize(AkU32 uScreenWidth, AkU32 uScreenHeight);
	
	IRenderer* GetRenderer() { return _pRenderer; }
	HWND GetHwnd() { return _hWnd; }
	AkF32 GetFps() { return _fFps; }
	UTimer* GetTimer() { return _pTimer; }
	UGameInput* GetGameInput() { return _pGameInput; }
	UCollisionManager* GetCollisionManager() { return _pCollisionManager; }
	USceneManager* GetSceneManager() { return _pSceneManager; }
	UEditorManager* GetEditorManager() { return _pEditorManager; }
	UAssetManager* GetAssetManager() { return _pAssetManager; }
	UAnimator* GetAnimator() { return _pAnimator; }
	UGameEventManager* GetGameEventManager() { return _pGameEventManager; }
	UEditorEvenetManager* GetEditorEventManager() { return _pEditorEventManager; }
	UModelManager* GetModelManager() { return _pModelManager; }
	USoundManager* GetSoundManager() { return _pSoundManager; }
	UUIManager* GetUIManager() { return _pUIManager; }
	AkBool EnableEditor() { return _bEnableEditor; }
	AkF32 GetClampNdcX() { return _fClampNdcX; }
	AkF32 GetClampNdcY() { return _fClampNdcY; }
	AkU32 GetScreenWidth() { return _uScreenWidth; }
	AkU32 GetScreenHeight() { return _uScreenHeight; }
	void SetVSync(AkBool bUseVSync);



	USound* GetTestSound() { return _pTestSound; }

private:
	void CleanUp();

	void Update(const UTimer* pTimer);
	void UpdateMouseNdcPos();
	void UpdateText();
	void Render(const UTimer* pTimer);
	void CalculateFrameRate();
	void ExitGame();

private:
	HWND _hWnd = nullptr;
	HMODULE _hRendererDLL = nullptr;
	IRenderer* _pRenderer = nullptr;
	UTimer* _pTimer = nullptr;
	UGameInput* _pGameInput = nullptr;
	AkF32 _fFps = 0.0f;
	AkU32 _uScreenWidth = 0;
	AkU32 _uScreenHeight = 0;
	AkF32 _fClampNdcX = 0.0f;
	AkF32 _fClampNdcY = 0.0f;
	UCollisionManager* _pCollisionManager = nullptr;
	USceneManager* _pSceneManager = nullptr;
	UEditorManager* _pEditorManager = nullptr;
	AkU32 _uEditorType = 0;
	AkBool _bUpdatedFirst = AK_TRUE;
	AkBool _bEnableEditor = AK_FALSE;
	UAssetManager* _pAssetManager = nullptr;
	UAnimator* _pAnimator = nullptr;
	UGameEventManager* _pGameEventManager = nullptr;
	UEditorEvenetManager* _pEditorEventManager = nullptr;
	UModelManager* _pModelManager = nullptr;
	UUIManager* _pUIManager = nullptr;
	UTextUI* _pSysTextUI = nullptr;
	UInputUI* _pDynamicTextUI = nullptr;
	Vector3 _vSysFontColor = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 _vDynamicTextFontColor = Vector3(1.0f);
	AkBool _bUseVSync = AK_FALSE;

	AkBool _bESC = AK_FALSE;

	USoundManager* _pSoundManager = nullptr;
	USound* _pTestSound = nullptr;

public:
	AkF32 _fNdcX = 0.0f;
	AkF32 _fNdcY = 0.0f;
};


