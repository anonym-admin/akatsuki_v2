#include "pch.h"
#include "Application.h"
#include "Timer.h"
#include "GameInput.h"
#include "SceneInGame.h"
#include "GeometryGenerator.h"
#include "CollisionManager.h"
#include "SceneManager.h"
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

Application::Application()
{
}

Application::~Application()
{
	CleanUp();
}

AkBool Application::InitApplication(AkBool bEnableDebugLayer, AkBool bEnableGBV)
{
	srand((AkU32)time(nullptr));

	if (!InitRenderer(bEnableDebugLayer, bEnableGBV))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pSysTextUI = new TextUI;
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

	_pDynamicTextUI = new InputUI;
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

	TextUI* pStaticTextUI = new TextUI;
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
	pBtnUI->SetClickFunc(&Application::ExitGame);

	pTextureUI->AddChildUI(pBtnUI);

	// Reset timer.
	GTimer->Reset();

	return AK_TRUE;
}

void Application::RunApplication()
{
	GTimer->Tick();

	Update(_pTimer);

	GRenderer->UpdateCascadeOrthoProjMatrix();

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

AkBool Application::UpdateWindowSize(AkU32 uScreenWidth, AkU32 uScreenHeight)
{
	AkBool bResult = AK_FALSE;

	if (GRenderer)
	{
		bResult = GRenderer->UpdateWindowSize(uScreenWidth, uScreenHeight);
	}

	return bResult;
}

void Application::SetVSync(AkBool bUseVSync)
{
	_bUseVSync = bUseVSync;

	GRenderer->SetVSync(_bUseVSync);
}

void Application::CleanUp()
{
	if (GRenderer)
	{
		GRenderer->Release();
		GRenderer = nullptr;
	}
	if (_hRendererDLL)
	{
#ifndef _DEBUG
		::FreeLibrary(_hRendererDLL);
		_hRendererDLL = nullptr;
#endif
	}
}

AkBool Application::InitRenderer(AkBool bEnableDebugLayer, AkBool bEnableGBV)
{
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

	IRenderer* pRenderer = nullptr;

	wchar_t wcErrorText[128] = {};
	AkU32 uErrorCode = 0;

	_hRendererDLL = ::LoadLibrary(wcRendererDLLFilename);
	if (!_hRendererDLL)
	{
		uErrorCode = ::GetLastError();
		swprintf_s(wcErrorText, L"Failed LoadLibrary[%s] - Error Code: %u \n", wcRendererDLLFilename, uErrorCode);
		::MessageBox(GhWnd, wcErrorText, L"Error", MB_OK);
		__debugbreak();
		return AK_FALSE;
	}

	DLL_CreateInstanceFuncPtr pDLL_CreateInstance = reinterpret_cast<DLL_CreateInstanceFuncPtr>(::GetProcAddress(_hRendererDLL, "DLL_CreateInstance"));
	pDLL_CreateInstance(reinterpret_cast<void**>(&pRenderer));

	if (!pRenderer->Initialize(GhWnd, bEnableDebugLayer, bEnableGBV))
	{
		__debugbreak();
		return AK_FALSE;
	}

	GRenderer = pRenderer;

	return AK_TRUE;
}

void Application::Update()
{
	static AkF32 fTimeElapsed = 0.0f;
	fTimeElapsed += DT;

	// Not Vsync => 60fps 고정을 위한 처리
	//if (fTimeElapsed < 0.016f)
	//{
	//	return;
	//}

	// Update game input.
	GGameInput->Update();

	// Update mouse ndc pos.
	UpdateMouseNdcPos();

	// Update Game Env.
	UpdateEnviroment();

	// Update Scene list.
	GSceneManager->Update();

	// Final Update Scene list.
	GSceneManager->FinalUpdate();

	// Update Collision manager.
	GCollisionManager->Update();

	// Update status text
	UpdateText();

	// Update UI Manager.
	GUIManager->Update();

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

void Application::UpdateMouseNdcPos()
{
	// AkI32 iMousePosX = _pGameInput->GetMouseX();
	AkI32 iMousePosX = _pGameInput->GetReleasedMouseX();
	AkI32 iMousePosY = _pGameInput->GetMouseY();

	_fNdcX = (AkF32)iMousePosX / _uScreenWidth * 2.0f - 1.0f;
	_fNdcY = (AkF32)iMousePosY / _uScreenHeight * -2.0f + 1.0f;

	_fClampNdcX = Clamp(_fNdcX, -1.0f, 1.0f);
	_fClampNdcY = Clamp(_fNdcY, -1.0f, 1.0f);
}

void Application::UpdateEnviroment()
{
	if (KEY_DOWN(KEY_INPUT_F10))
	{
		_bUseVSync = !_bUseVSync;

		SetVSync(_bUseVSync);
	}
	// TODO!!
	// 임시로 F1 버튼으로 설정.
	if (KEY_DOWN(KEY_INPUT_F1))
	{
		GUIManager->ToggleUI(UI_OBJECT_TYPE::UI_OBJ_EXIT);
	}
}

void Application::UpdateText()
{
	SceneInGame* pSceneInGame = (SceneInGame*)_pSceneManager->GetScene(SCENE_TYPE::SCENE_TYPE_INGANE);

	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;
	wchar_t wcText[128] = {};
	AkU32 uTxtLen = swprintf_s(wcText, L"fps:%.2lf tri:%u obj:%u/%u\n", _fFps, pSceneInGame->GetTriangleCount(), pSceneInGame->GetRenderMapObjectCount(), pSceneInGame->GetMapObjectCount());

	_pSysTextUI->WriteText(wcText);
}

void Application::Render(const Timer* pTimer)
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

void Application::CalculateFrameRate()
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

void Application::ExitGame()
{
	if (_bEnableEditor)
	{
		_pEditorManager->GetCurrentEditor()->EndEditor();
	}

	::PostQuitMessage(996);
}


