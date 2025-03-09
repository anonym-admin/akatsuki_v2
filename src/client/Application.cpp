#include "pch.h"
#include "Application.h"
#include "Camera.h"
#include "TextUI.h"
#include "PanelUI.h"
#include "BtnUI.h"
#include "InputUI.h"

#include "SceneInGame.h"
#include "SceneLoading.h"
#include "EditorModel.h"
#include "EditorMap.h"

#include "Collider.h"

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

	// Init Rederer dll.
	if (!InitRenderer(bEnableDebugLayer, bEnableGBV))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Init Scene.
	if (!InitScene())
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Init Editor.
	if (!InitEditor())
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Init Game UI.
	if (!InitUI())
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Reset timer.
	GTimer->Reset();

	return AK_TRUE;
}

void Application::RunApplication()
{
	GTimer->Tick();

	//// Start the Dear ImGui frame
	//ImGui_ImplDX12_NewFrame();
	//ImGui_ImplWin32_NewFrame();
	//ImGui::NewFrame();
	//ImGuiIO& io = ImGui::GetIO();

	// Update.
	Update();

	GRenderer->UpdateCascadeOrthoProjMatrix();

	// Shadow Pass.
	for (AkU32 i = 0; i < 5; i++)
	{
		GRenderer->BeginCasterRenderPreparation();
		GEditorManager->RenderShadow();
		GSceneManager->RenderShadow();
		GRenderer->EndCasterRenderPreparation();
	}

	// Begin render.
	GRenderer->BeginRender();

	// Render.
	Render();

	//// ImGui Render.
	//ImGui::Render();

	// End render.
	GRenderer->EndRender();
	GRenderer->Present();

	CalculateFrameRate();

	// Excute Event Manager.
	GEventManager->Excute();
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

void Application::CleanUp()
{
	// GRenderer->UnBindImGui();

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

	// GRenderer->BindImGui((void**)&GImGui);

	return AK_TRUE;
}

AkBool Application::InitScene()
{
	GSceneManager->AddScene(SCENE_TYPE::SCENE_TYPE_LOADING, new SceneLoading());
	GSceneManager->AddScene(SCENE_TYPE::SCENE_TYPE_INGANE, new SceneInGame());

	GSceneManager->BindCurrentScene(SCENE_TYPE::SCENE_TYPE_LOADING)->BeginScene();

	return AK_TRUE;
}

AkBool Application::InitEditor()
{
	GEditorManager->AddEditor(EDITOR_TYPE::EDITOR_TYPE_MODEL, new EditorModel());
	GEditorManager->AddEditor(EDITOR_TYPE::EDITOR_TYPE_MAP, new EditorMap());

	// Editor 들은 모두 Begin 진행.
	GEditorManager->GetEditor(EDITOR_TYPE::EDITOR_TYPE_MODEL)->BeginEditor();
	GEditorManager->GetEditor(EDITOR_TYPE::EDITOR_TYPE_MAP)->BeginEditor();

	return AK_TRUE;
}

AkBool Application::InitUI()
{
	_pSysTextUI = new TextUI(256, 32, L"Consolas", 10);
	_pSysTextUI->SetPosition(10, 10);
	_pSysTextUI->SetScale(1.0f, 1.0f);
	_pSysTextUI->SetFontColor(&_vSysFontColor);

	GUIManager->AddUI(_pSysTextUI, UI_TYPE::UI_OBJ_SYS_INFO_TEXT);
	GUIManager->OnUI(UI_TYPE::UI_OBJ_SYS_INFO_TEXT);

	_pDynamicTextUI = new InputUI(256, 32, L"Consolas", 10);
	_pDynamicTextUI->SetPosition(10, 500);
	_pDynamicTextUI->SetScale(1.0f, 1.0f);
	_pDynamicTextUI->SetFontColor(&_vDynamicTextFontColor);
	_pDynamicTextUI->SetDrawBackGround(AK_TRUE);

	GUIManager->AddUI(_pDynamicTextUI, UI_TYPE::UI_OBJ_CHAT_INPUT_TEXT);
	GUIManager->OnUI(UI_TYPE::UI_OBJ_CHAT_INPUT_TEXT);

	TextUI* pStaticTextUI = new TextUI(256, 32, L"Consolas", 10);
	pStaticTextUI->SetPosition(10, 32 + 10 + 10);
	pStaticTextUI->SetScale(1.0f, 1.0f);
	pStaticTextUI->SetFontColor(&_vSysFontColor);
	pStaticTextUI->WriteText(L"Test Static Text\n");

	GUIManager->AddUI(pStaticTextUI, UI_TYPE::UI_OBJ_TEST_STATIC_TEXT);

	UPanelUI* pTextureUI = new UPanelUI(L"../../assets/ui_01.dds", 0, 0, 2545, 1867);
	pTextureUI->SetPosition(500, 10);
	pTextureUI->SetScale(0.1f, 0.2f);
	pTextureUI->SetDrawBackGround(AK_TRUE);
	pTextureUI->SetResolution((AkU32)(0.1f * 2545), (AkU32)(0.2f * 1867));

	GUIManager->AddUI(pTextureUI, UI_TYPE::UI_OBJ_EXIT);

	UBtnUI* pBtnUI = new UBtnUI(L"../../assets/Exit_Btn.dds", 0, 0, 225, 49);
	pBtnUI->SetRelativePosition(10, 10);
	pBtnUI->SetScale(1.0f, 1.0f);
	pBtnUI->SetDrawBackGround(AK_TRUE);
	pBtnUI->SetResolution(225, 49);
	pBtnUI->SetClickFunc(&Application::ExitGame);

	pTextureUI->AddChildUI(pBtnUI);

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

	// Update Game Env.
	UpdateEnviroment();

	// Update Editor list.
	GEditorManager->Update();

	// Final Update Editor list.
	GEditorManager->FinalUpdate();

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

	// Exit Game.
	if (KEY_DOWN(KEY_INPUT_ESCAPE))
	{
		ExitGame();
	}
}

void Application::UpdateEnviroment()
{
	if (KEY_DOWN(KEY_INPUT_F10))
	{
		_bUseVSync = !_bUseVSync;

		GRenderer->SetVSync(_bUseVSync);
	}
	if (KEY_DOWN(KEY_INPUT_F5))
	{
		_bChangeEditor = !_bChangeEditor;

		EventHandle_t tEvent = {};
		if (_bChangeEditor)
		{
			tEvent.eEventType = EVENT_TYPE::SCENE_TO_EDITOR_CHANGE;
			tEvent.tSceneAndEditorChangeParam.eAfterEditor = EDITOR_TYPE::EDITOR_TYPE_MODEL;
		}
		else
		{
			tEvent.eEventType = EVENT_TYPE::EDITOR_TO_SCENE_CHANGE;
			tEvent.tSceneAndEditorChangeParam.eAfterScene = SCENE_TYPE::SCENE_TYPE_INGANE;
		}
		GEventManager->AddEvent(&tEvent);
	}
	if (KEY_DOWN(KEY_INPUT_F2))
	{
		New_Collider::DRAW_COLLIDER = !New_Collider::DRAW_COLLIDER;
	}

	// TODO!!
	// 임시로 F1 버튼으로 설정.
	if (KEY_DOWN(KEY_INPUT_F1))
	{
		GUIManager->ToggleUI(UI_TYPE::UI_OBJ_EXIT);
	}
}

void Application::UpdateText()
{
	SceneInGame* pSceneInGame = (SceneInGame*)GSceneManager->GetScene(SCENE_TYPE::SCENE_TYPE_INGANE);

	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;
	wchar_t wcText[128] = {};
	AkU32 uTxtLen = swprintf_s(wcText, L"fps:%.2lf tri:%u obj:%u/%u\n", _fFps, pSceneInGame->GetTriangleCount(), pSceneInGame->GetRenderMapObjectCount(), pSceneInGame->GetMapObjectCount());

	_pSysTextUI->WriteText(wcText);
}

void Application::Render()
{
	// Render Editor list.
	GEditorManager->Render();

	// Render Scene list.
	GSceneManager->Render();

	// Render UI
	GUIManager->Render();
}

void Application::CalculateFrameRate()
{
	static AkU32 uFrameCount = 0;
	static AkF32 fTimeElapsed = 0.0f;

	uFrameCount++;

	if (GTimer->GetTotalTime() - fTimeElapsed >= 1.0f)
	{
		AkF32 fFps = static_cast<AkF32>(uFrameCount);

		_fFps = fFps;

		uFrameCount = 0;
		fTimeElapsed += 1.0f;
	}
}

void Application::ExitGame()
{
	::PostQuitMessage(996);
}


