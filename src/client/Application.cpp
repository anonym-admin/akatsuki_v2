#include "pch.h"
#include "Application.h"
#include "Timer.h"
#include "GameInput.h"
#include "SceneInGame.h"
#include "GeometryGenerator.h"
#include "CollisionManager.h"
#include "SceneManager.h"
#include "EditorManager.h"
#include "AssetManager.h"
#include "Animation.h"
#include "EventManager.h"
#include "ModelManager.h"
#include "UIManager.h"
#include "Editor.h"
#include "Camera.h"
#include "TextUI.h"
#include "PanelUI.h"
#include "BtnUI.h"
#include "InputUI.h"

#include "Sound.h"

/*
===============
Application
===============
*/

UApplication::UApplication()
{
}

UApplication::~UApplication()
{
	CleanUp();
}

AkBool UApplication::InitApplication(HWND hWnd, AkBool bEnableDebugLayer, AkBool bEnableGBV)
{
	srand((AkU32)time(nullptr));

	const wchar_t* wcRendererDLLFilename = nullptr;

#if defined(_M_AMD64)

#if defined(_DEBUG) || defined(DEBUG)
	wcRendererDLLFilename = L"akatsuki_renderer_x64d.dll";
#else
	wcRendererDLLFilename = L"akatsuki_renderer_x64.dll";
#endif

#elif defined(_M_IX86)

#if defined(_DEBUG) | defined(DEBUG)
	wcRendererDLLFilename = L"akatsuki_renderer_x86d.dll";
#else
	wcRendererDLLFilename = L"akatsuki_renderer_x86.dll";
#endif

#endif

	wchar_t wcErrorText[128] = {};
	AkU32 uErrorCode = 0;

	_hRendererDLL = ::LoadLibrary(wcRendererDLLFilename);
	if (!_hRendererDLL)
	{
		uErrorCode = ::GetLastError();
		swprintf_s(wcErrorText, L"Failed LoadLibrary[%s] - Error Code: %u \n", wcRendererDLLFilename, uErrorCode);
		::MessageBox(hWnd, wcErrorText, L"Error", MB_OK);
		__debugbreak();
	}

	DLL_CreateInstanceFuncPtr pDLL_CreateInstance = reinterpret_cast<DLL_CreateInstanceFuncPtr>(::GetProcAddress(_hRendererDLL, "DLL_CreateInstance"));
	pDLL_CreateInstance(reinterpret_cast<void**>(&_pRenderer));

	if (!_pRenderer->Initialize(hWnd, bEnableDebugLayer, bEnableGBV))
	{
		__debugbreak();
	}

	_hWnd = hWnd;

	RECT tRect = {};
	::GetClientRect(_hWnd, &tRect);
	_uScreenWidth = tRect.right - tRect.left;
	_uScreenHeight = tRect.bottom - tRect.top;

	_pTimer = new UTimer;
	if (!_pTimer->Initialize())
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pGameInput = new UGameInput;
	if (!_pGameInput->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pCollisionManager = new UCollisionManager;
	if (!_pCollisionManager->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pAssetManager = new UAssetManager;
	if (!_pAssetManager->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pAnimator = new UAnimator;
	if (!_pAnimator->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pGameEventManager = new UGameEventManager;
	if (!_pGameEventManager->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pEditorEventManager = new UEditorEvenetManager;
	if (!_pEditorEventManager->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pModelManager = new UModelManager;
	if (!_pModelManager->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pSceneManager = new USceneManager;
	if (!_pSceneManager->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pEditorManager = new UEditorManager;
	if (!_pEditorManager->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pUIManager = new UUIManager;
	if (!_pUIManager->Initialize(this))
	{
		__debugbreak();
		return AK_FALSE;
	}

	/////////////////////////////////////////////////////////////
	_pSoundManager = new USoundManager;
	if (!_pSoundManager->Initialize())
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pTestSound = _pSoundManager->LoadSound("Fire_Continue.mp3");
	/////////////////////////////////////////////////////////////

	_pSysTextUI = new UTextUI;
	if (!_pSysTextUI->Initialize(this, 256, 32, L"Consolas", 10))
	{
		__debugbreak();
		return AK_FALSE;
	}
	_pSysTextUI->SetPosition(10, 10);
	_pSysTextUI->SetScale(1.0f, 1.0f);
	_pSysTextUI->SetFontColor(&_vSysFontColor);

	_pUIManager->AddUI(_pSysTextUI, UI_OBJECT_TYPE::UI_OBJ_SYS_INFO_TEXT);
	_pUIManager->OnUI(UI_OBJECT_TYPE::UI_OBJ_SYS_INFO_TEXT);

	_pDynamicTextUI = new UInputUI;
	if (!_pDynamicTextUI->Initialize(this, 256, 32, L"Consolas", 10))
	{
		__debugbreak();
		return AK_FALSE;
	}
	_pDynamicTextUI->SetPosition(10, 500);
	_pDynamicTextUI->SetScale(1.0f, 1.0f);
	_pDynamicTextUI->SetFontColor(&_vDynamicTextFontColor);
	_pDynamicTextUI->SetDrawBackGround(AK_TRUE);

	_pUIManager->AddUI(_pDynamicTextUI, UI_OBJECT_TYPE::UI_OBJ_CHAT_INPUT_TEXT);
	_pUIManager->OnUI(UI_OBJECT_TYPE::UI_OBJ_CHAT_INPUT_TEXT);

	UTextUI* pStaticTextUI = new UTextUI;
	if (!pStaticTextUI->Initialize(this, 256, 32, L"Consolas", 10))
	{
		__debugbreak();
		return AK_FALSE;
	}
	pStaticTextUI->SetPosition(10, 32 + 10 + 10);
	pStaticTextUI->SetScale(1.0f, 1.0f);
	pStaticTextUI->SetFontColor(&_vSysFontColor);
	pStaticTextUI->WriteText(L"Test Static Text\n");

	_pUIManager->AddUI(pStaticTextUI, UI_OBJECT_TYPE::UI_OBJ_TEST_STATIC_TEXT);

	UPanelUI* pTextureUI = new UPanelUI;
	if (!pTextureUI->Initialize(this, L"../../assets/ui_01.dds", 0, 0, 2545, 1867))
	{
		__debugbreak();
		return AK_FALSE;
	}
	pTextureUI->SetPosition(500, 10);
	pTextureUI->SetScale(0.1f, 0.2f);
	pTextureUI->SetDrawBackGround(AK_TRUE);
	pTextureUI->SetResolution((AkU32)(0.1f * 2545), (AkU32)(0.2f * 1867));

	_pUIManager->AddUI(pTextureUI, UI_OBJECT_TYPE::UI_OBJ_EXIT);

	UBtnUI* pBtnUI = new UBtnUI;
	if (!pBtnUI->Initialize(this, L"../../assets/Exit_Btn.dds", 0, 0, 225, 49))
	{
		__debugbreak();
		return AK_FALSE;
	}
	pBtnUI->SetRelativePosition(10, 10);
	pBtnUI->SetScale(1.0f, 1.0f);
	pBtnUI->SetDrawBackGround(AK_TRUE);
	pBtnUI->SetResolution(225, 49);
	pBtnUI->SetClickFunc(&UApplication::ExitGame);

	pTextureUI->AddChildUI(pBtnUI);

	// Reset timer.
	_pTimer->Reset();

	return AK_TRUE;
}

void UApplication::RunApplication()
{
	_pTimer->Tick();

	Update(_pTimer);

	_pRenderer->UpdateCascadeOrthoProjMatrix();

	// Shadow Pass.
	for (AkU32 i = 0; i < 5; i++)
	{
		_pRenderer->BeginCasterRenderPreparation();
		_pSceneManager->RenderShadowPass();
		_pRenderer->EndCasterRenderPreparation();
	}

	// Begin render.
	_pRenderer->BeginRender();

	Render(_pTimer);

	// End render.
	_pRenderer->EndRender();
	_pRenderer->Present();

	CalculateFrameRate();

	_pGameEventManager->Reset();
	_pEditorEventManager->Reset();
}

AkBool UApplication::UpdateWindowSize(AkU32 uScreenWidth, AkU32 uScreenHeight)
{
	AkBool bResult = AK_FALSE;

	if (_pRenderer)
	{
		bResult = _pRenderer->UpdateWindowSize(uScreenWidth, uScreenHeight);
		_uScreenWidth = uScreenWidth;
		_uScreenHeight = uScreenHeight;
	}

	return bResult;
}

void UApplication::SetVSync(AkBool bUseVSync)
{
	_bUseVSync = bUseVSync;

	_pRenderer->SetVSync(_bUseVSync);
}

void UApplication::CleanUp()
{
	if (_pSoundManager)
	{
		delete _pSoundManager;
		_pSoundManager = nullptr;
	}
	if (_pUIManager)
	{
		delete _pUIManager;
		_pUIManager = nullptr;
	}
	if (_pEditorEventManager)
	{
		delete _pEditorEventManager;
		_pEditorEventManager = nullptr;
	}
	if (_pGameEventManager)
	{
		delete _pGameEventManager;
		_pGameEventManager = nullptr;
	}
	if (_pEditorManager)
	{
		delete _pEditorManager;
		_pEditorManager = nullptr;
	}
	if (_pSceneManager)
	{
		delete _pSceneManager;
		_pSceneManager = nullptr;
	}
	if (_pModelManager)
	{
		delete _pModelManager;
		_pModelManager = nullptr;
	}
	if (_pAnimator)
	{
		delete _pAnimator;
		_pAnimator = nullptr;
	}
	if (_pAssetManager)
	{
		delete _pAssetManager;
		_pAssetManager = nullptr;
	}
	if (_pCollisionManager)
	{
		delete _pCollisionManager;
		_pCollisionManager = nullptr;
	}
	if (_pGameInput)
	{
		delete _pGameInput;
		_pGameInput = nullptr;
	}
	if (_pTimer)
	{
		delete _pTimer;
		_pTimer = nullptr;
	}
	if (_pRenderer)
	{
		_pRenderer->Release();
		_pRenderer = nullptr;
	}
	if (_hRendererDLL)
	{
#ifndef _DEBUG
		::FreeLibrary(_hRendererDLL);
		_hRendererDLL = nullptr;
#endif
	}
}

void UApplication::Update(const UTimer* pTimer)
{
	static AkF32 fTimeElapsed = 0.0f;
	AkF32 fDeltaTime = pTimer->GetDeltaTime();

	fTimeElapsed += fDeltaTime;

	// Not Vsync => 60fps 고정을 위한 처리
	//if (fTimeElapsed < 0.016f)
	//{
	//	return;
	//}

	// Update game input.
	_pGameInput->Update();

	// Update mouse ndc pos.
	UpdateMouseNdcPos();

	if (_pGameInput->KeyFirstDown(KEY_INPUT_RSHIFT))
	{
		if (_bUpdatedFirst)
		{
			_bEnableEditor = AK_TRUE;

			_pEditorManager->GetCurrentEditor()->BeginEditor();
		}
		else
		{
			EDITOR_TYPE eEditorType = (EDITOR_TYPE)_uEditorType;

			if (EDITOR_TYPE::EDITOR_TYPE_COUNT == eEditorType)
			{
				_bEnableEditor = AK_FALSE;

				// 종료 타입 이전 단계의 에티터 타입을 종료
				AkU32 uBeforeExitType = (AkU32)EDITOR_TYPE::EDITOR_TYPE_COUNT - 1;

				_pEditorManager->GetEditor((EDITOR_TYPE)uBeforeExitType)->EndEditor();
			}
			else
			{
				_bEnableEditor = AK_TRUE;

				EditorEventHandle_t tEditorEventHandle = {};
				tEditorEventHandle.eEventType = EDITOR_EVENT_TYPE::EDITOR_EVENT_TYPE_EDITOR_CHANGE;
				tEditorEventHandle.tEditorChangeParam.eAfter = eEditorType;

				_pEditorEventManager->AddEvent(&tEditorEventHandle);

				// _pEditorManager->ChangeEditor(eEditorType);
			}
		}

		_uEditorType++;
		_uEditorType %= ((AkU32)EDITOR_TYPE::EDITOR_TYPE_COUNT + 1);

		_bUpdatedFirst = AK_FALSE;
	}

	if (_pGameInput->KeyFirstDown(KEY_INPUT_F5))
	{
		_bUseVSync = !_bUseVSync;

		SetVSync(_bUseVSync);
	}

	// TODO!!
	// 임시로 F1 버튼으로 설정.
	if (_pGameInput->KeyFirstDown(KEY_INPUT_F1))
	{
		_pUIManager->ToggleUI(UI_OBJECT_TYPE::UI_OBJ_EXIT);
	}

	if (_bEnableEditor)
	{
		_pEditorManager->GetCurrentEditor()->Update(fDeltaTime);
	}

	// Update Scene list.
	_pSceneManager->Update(fDeltaTime);

	// Final Update Scene list.
	_pSceneManager->FinalUpdate(fDeltaTime);

	// Update Collision manager.
	_pCollisionManager->Update();

	// Update status text
	UpdateText();

	// Update UI Manager.
	_pUIManager->Update(fDeltaTime);

	// Excute event manager.
	_pGameEventManager->Excute(fDeltaTime);
	_pEditorEventManager->Excute(fDeltaTime);

	// 게임 종료.
	if (_pGameInput->KeyFirstDown(KEY_INPUT_ESCAPE))
	{
		ExitGame();
	}

	// Update sound manager.
	_pSoundManager->Update(fDeltaTime);
	// _pTestSound->Play(AK_TRUE);

	fTimeElapsed = 0.0f;
}

void UApplication::UpdateMouseNdcPos()
{
	// AkI32 iMousePosX = _pGameInput->GetMouseX();
	AkI32 iMousePosX = _pGameInput->GetReleasedMouseX();
	AkI32 iMousePosY = _pGameInput->GetMouseY();

	_fNdcX = (AkF32)iMousePosX / _uScreenWidth * 2.0f - 1.0f;
	_fNdcY = (AkF32)iMousePosY / _uScreenHeight * -2.0f + 1.0f;

	_fClampNdcX = Clamp(_fNdcX, -1.0f, 1.0f);
	_fClampNdcY = Clamp(_fNdcY, -1.0f, 1.0f);
}

void UApplication::UpdateText()
{
	USceneInGame* pSceneInGame = (USceneInGame*)_pSceneManager->GetScene(GAME_SCENE_TYPE::SCENE_TYPE_INGANE);

	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;
	wchar_t wcText[128] = {};
	AkU32 uTxtLen = swprintf_s(wcText, L"fps:%.2lf tri:%u obj:%u/%u\n", _fFps, pSceneInGame->GetTriangleCount(), pSceneInGame->GetRenderMapObjectCount(), pSceneInGame->GetMapObjectCount());

	_pSysTextUI->WriteText(wcText);
}

void UApplication::Render(const UTimer* pTimer)
{
	// Render editor.
	if (_bEnableEditor)
	{
		_pEditorManager->GetCurrentEditor()->Render();
	}

	// Render Scene list.
	_pSceneManager->Render();

	// Render UI
	_pUIManager->Render();

	if (_bESC)
	{
	}
}

void UApplication::CalculateFrameRate()
{
	static AkU32 uFrameCount = 0;
	static AkF32 fTimeElapsed = 0.0f;

	uFrameCount++;

	if (_pTimer->GetTotalTime() - fTimeElapsed >= 1.0f)
	{
		AkF32 fFps = static_cast<AkF32>(uFrameCount);

		_fFps = fFps;

		uFrameCount = 0;
		fTimeElapsed += 1.0f;
	}
}

void UApplication::ExitGame()
{
	if (_bEnableEditor)
	{
		_pEditorManager->GetCurrentEditor()->EndEditor();
	}

	::PostQuitMessage(996);
}


