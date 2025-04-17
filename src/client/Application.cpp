#include "pch.h"
#include "Application.h"
#include "Camera.h"
#include "SceneInGame.h"
#include "SceneLoading.h"
#include "EditorModel.h"
#include "EditorMap.h"
#include "EditorParticle.h"
#include "Collider.h"
#include "Terrain.h"
#include "PostRenderControl.h"

#include "Sound.h"

#include "Model.h"
#include "UIImage.h"
#include "UIButton.h"

#include "FrustumCulling.h"

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

	if (!InitTextResource())
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Create Post process
	_pPostProcess = new PostRenderControl;

	// Reset timer.
	GTimer->Reset();

	// Bind ImGui.
	GRenderer->BindImGui((void**)&GImGui);

	return AK_TRUE;
}

void Application::RunApplication()
{
	/*
	========================
	Update
	========================
	*/

	GTimer->Tick();

	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuiIO& io = ImGui::GetIO();

	// Update.
	Update();

	// Update Text.
	UpdateText();

	GRenderer->SetTotalTime(GTimer->GetTotalTime());

	// Update Shadow Map Matrix
	GRenderer->UpdateCascadeOrthoProjMatrix();

	/*
	========================
	RENDER
	========================
	*/

	// Render Depth Map.
	GRenderer->BeginRenderDepthMap();

	GSceneManager->RenderDepthMap();

	GRenderer->EndRenderDepthMap();

	// Render Shadow Map.
	for (AkU32 i = 0; i < 5; i++)
	{
		GRenderer->BeginRenderShadowMaps();

		GEditorManager->RenderShadowMaps();
		GSceneManager->RenderShadowMaps();

		GRenderer->EndRenderShadowMaps();
	}

	// Begin render.
	GRenderer->BeginRender();

	// Render.
	Render();

	// Render Text.
	RenderText();

	// Post Process Control.
	if (_bUsePostProcessController)
	{
		_pPostProcess->RendeGUI();
	}

	// ImGui Render.
	ImGui::Render();

	// End render.
	GRenderer->EndRender();
	GRenderer->Present();

	CalculateFrameRate();

	// Select Editor.
	if (_bChangeEditor && !_bPlayingEditor)
	{
		wscanf_s(L"%d", &_iEditorType);

		EventHandle_t tEvent = {};
		switch (_iEditorType)
		{
		case 0:
		{
			tEvent.eEventType = EVENT_TYPE::SCENE_TO_EDITOR_CHANGE;
			tEvent.tSceneAndEditorChangeParam.eAfterEditor = EDITOR_TYPE::EDITOR_MODEL;
		}
		break;
		case 1:
		{
			tEvent.eEventType = EVENT_TYPE::SCENE_TO_EDITOR_CHANGE;
			tEvent.tSceneAndEditorChangeParam.eAfterEditor = EDITOR_TYPE::EDITOR_MAP;
		}
		break;
		case 2:
		{
			tEvent.eEventType = EVENT_TYPE::SCENE_TO_EDITOR_CHANGE;
			tEvent.tSceneAndEditorChangeParam.eAfterEditor = EDITOR_TYPE::EDITOR_PARTICLE;
		}
		break;
		}

		GEventManager->AddEvent(&tEvent);
		_bPlayingEditor = AK_TRUE;
	}

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
	if (_pScreenTextFontObj)
	{
		GRenderer->DestroyFontObject(_pScreenTextFontObj);
		_pScreenTextFontObj = nullptr;
	}
	if (_pScreenTextureHandle)
	{
		GRenderer->DestroyTexture(_pScreenTextureHandle);
		_pScreenTextureHandle = nullptr;
	}
	if (_pScreenTextureImage)
	{
		delete[] _pScreenTextureImage;
		_pScreenTextureImage = nullptr;
	}
	if (_pTextTextureHandle)
	{
		GRenderer->DestroyTexture(_pTextTextureHandle);
		_pTextTextureHandle = nullptr;
	}
	if (_pTextTextureImage)
	{
		delete[] _pTextTextureImage;
		_pTextTextureImage = nullptr;
	}
	if (_pPostProcess)
	{
		delete _pPostProcess;
		_pPostProcess = nullptr;
	}
	if (GImGui)
	{
		GRenderer->UnBindImGui();
		GImGui = nullptr;
	}
	if (GFont)
	{
		GRenderer->DestroyFontObject(GFont);
		GFont = nullptr;
	}
	if (GSprite)
	{
		GSprite->Release();
		GSprite = nullptr;
	}
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

	// Create Common Sprite Obj.
	GSprite = GRenderer->CreateSpriteObject();

	// Create Font Obj.
	GFont = GRenderer->CreateFontObject(SYSTEM_FONT_FAMILY_NAME, SYSTEM_FONT_SIZE);

	// Set VSync
	GRenderer->SetVSync(_bUseVSync);

	return AK_TRUE;
}

AkBool Application::InitScene()
{
	GSceneManager->AddScene(SCENE_TYPE::LOADING, new SceneLoading());
	GSceneManager->AddScene(SCENE_TYPE::INGANE, new SceneInGame());

	GSceneManager->BindCurrentScene(SCENE_TYPE::LOADING);

	return AK_TRUE;
}

AkBool Application::InitEditor()
{
	GEditorManager->AddEditor(EDITOR_TYPE::EDITOR_MODEL, new EditorModel());
	GEditorManager->AddEditor(EDITOR_TYPE::EDITOR_MAP, new EditorMap());
	GEditorManager->AddEditor(EDITOR_TYPE::EDITOR_PARTICLE, new EditorParticle());

	// Editor 들은 모두 Begin 진행.
	GEditorManager->GetEditor(EDITOR_TYPE::EDITOR_MODEL)->BeginEditor();
	GEditorManager->GetEditor(EDITOR_TYPE::EDITOR_MAP)->BeginEditor();
	GEditorManager->GetEditor(EDITOR_TYPE::EDITOR_PARTICLE)->BeginEditor();

	return AK_TRUE;
}

AkBool Application::InitUI()
{
	return AK_TRUE;
}

AkBool Application::InitTextResource()
{
	{
		_uTextTextureWidth = 512;
		_uTextTextureHeight = 512;

		_pTextTextureImage = new AkU8[_uTextTextureWidth * _uTextTextureHeight * 4];
		memset(_pTextTextureImage, 0, _uTextTextureWidth * _uTextTextureHeight * 4);

		_pTextTextureHandle = GRenderer->CreateDynamicTexture(_uTextTextureWidth, _uTextTextureHeight);
	}

	{
		RECT tRect = {};
		GetClientRect(GhWnd, &tRect);
		_uScreenTextureWidth = tRect.right - tRect.left;
		_uScreenTextureHeight = tRect.bottom - tRect.top;

		_pScreenTextureImage = new AkU8[_uScreenTextureWidth * _uScreenTextureHeight * 4];
		memset(_pScreenTextureImage, 0, _uScreenTextureWidth * _uScreenTextureHeight * 4);

		_pScreenTextureHandle = GRenderer->CreateDynamicTexture(_uScreenTextureWidth, _uScreenTextureHeight);
	}


	return AK_TRUE;
}

void Application::Update()
{
	static AkF32 fTimeElapsed = 0.0f;
	fTimeElapsed += GTimer->GetDeltaTime();

	// Not Vsync => 60fps 고정을 위한 처리
	if (AK_FALSE == _bUseVSync && fTimeElapsed < 0.016f)
	{
		return;
	}

	if(!_bUseVSync)
	{
		GDeltaTime = fTimeElapsed;
	}
	else
	{
		GDeltaTime = GTimer->GetDeltaTime();
	}

	// Update game input.
	GGameInput->Update();

	// Update Game Env.
	UpdateEnviroment();

	// Update Editor list.
	GEditorManager->Update();

	// Update Scene list.
	GSceneManager->Update();

	// Final Update Scene list.
	GSceneManager->FinalUpdate();

	// Update Collision manager.
	GCollisionManager->Update();

	// Update UI Manager.
	GUIManager->Update();

	// Exit Game.
	if (KEY_DOWN(KEY_INPUT_ESCAPE))
	{
		ExitGame();
	}

	fTimeElapsed = 0.0f;
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
			_pScreenTextFontObj = GRenderer->CreateFontObject(SYSTEM_FONT_FAMILY_NAME, 16);
		}
		else
		{
			GRenderer->DestroyFontObject(_pScreenTextFontObj);
			_pScreenTextFontObj = nullptr;

			tEvent.eEventType = EVENT_TYPE::EDITOR_TO_SCENE_CHANGE;
			tEvent.tSceneAndEditorChangeParam.eAfterScene = SCENE_TYPE::INGANE;

			_bPlayingEditor = AK_FALSE;
		}
		GEventManager->AddEvent(&tEvent);
	}
	if (KEY_DOWN(KEY_INPUT_F6))
	{
		EventHandle_t tEvent = {};
		if (!_bChangeEditor)
		{
			tEvent.eEventType = EVENT_TYPE::SCENE_CHANGE;
			tEvent.tSceneAndEditorChangeParam.eAfterScene = SCENE_TYPE::COMPUTE;
		}
		GEventManager->AddEvent(&tEvent);
	}
	// For Debug
	if (KEY_DOWN(KEY_INPUT_F2))
	{
		Collider::DRAW_COLLIDER = !Collider::DRAW_COLLIDER;
		Terrain::DRAW_WIRE = !Terrain::DRAW_WIRE;
		_bUseDebugMode = AK_TRUE;
	}
	if (_bUseDebugMode)
	{
		if (KEY_DOWN(KEY_INPUT_O))
		{
			_bUsePostProcessController = !_bUsePostProcessController;
			Camera::UPDATE_CAMERA = !Camera::UPDATE_CAMERA;
		}
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
	if(!_bChangeEditor)
	{
		SceneInGame* pSceneInGame = (SceneInGame*)GSceneManager->GetScene(SCENE_TYPE::INGANE);
		Actor* pPlayer = nullptr;
		Vector3 vPlayerPos = Vector3(0.0f);
		AkU32 uRenderObj = 0;
		AkU32 uTotalObj = 0;

		if (pSceneInGame)
		{
			GameObjContainer_t* pPlayerGroup = pSceneInGame->GetGroupObject(GAME_OBJECT_GROUP_TYPE::PLAYER);
			if (pPlayerGroup)
			{
				pPlayer = (Actor*)pPlayerGroup->pGameObjHead->pData;
				vPlayerPos = pPlayer->GetTransform()->GetPosition();
			}
			
			FrustumCulling* pFrustumCulling = pSceneInGame->GetFrustumCulling();
			if (pFrustumCulling)
			{
				uTotalObj = pFrustumCulling->GetTotalRenderObjCount();
				uRenderObj = uTotalObj - pFrustumCulling->GetCullObjCount();
			}
		}

		AkI32 iTextWidth = 0;
		AkI32 iTextHeight = 0;
		wchar_t wcText[256] = {};
		AkU32 uTxtLen = swprintf_s(wcText, L"fps:%.2lf vsync:%s\npos:%lf %lf %lf\nrender:%u/%u\n", GFps, _bUseVSync ? L"on" : L"off", vPlayerPos.x, vPlayerPos.y, vPlayerPos.z, uRenderObj, uTotalObj);

		if (wcscmp(_wcText, wcText))
		{
			memset(_pTextTextureImage, 0, _uTextTextureWidth * _uTextTextureHeight * 4);
			GRenderer->WriteTextToBitmap(_pTextTextureImage, _uTextTextureWidth, _uTextTextureHeight, _uTextTextureHeight * 4, &iTextWidth, &iTextHeight, GFont, wcText, uTxtLen);
			GRenderer->UpdateTextureWidthImage(_pTextTextureHandle, _pTextTextureImage, _uTextTextureWidth, _uTextTextureHeight);

			wcscpy_s(_wcText, wcText);
		}
	}
	else
	{
		if(!_bPlayingEditor)
		{
			AkI32 iTextWidth = 0;
			AkI32 iTextHeight = 0;
			wchar_t wcText[256] = {};
			AkU32 uTxtLen = swprintf_s(wcText, L"Select Editor Mode\n0. Model\n1. Map\n2. Particle\n");

			if (wcscmp(_wcText, wcText))
			{
				memset(_pScreenTextureImage, 0, _uScreenTextureWidth * _uScreenTextureHeight * 4);
				GRenderer->WriteTextToBitmap(_pScreenTextureImage, _uScreenTextureWidth, _uScreenTextureHeight, _uScreenTextureWidth * 4, &iTextWidth, &iTextHeight, _pScreenTextFontObj, wcText, uTxtLen, FONT_COLOR_TYPE::FONT_COLOR_TYPE_GREEN);
				GRenderer->UpdateTextureWidthImage(_pScreenTextureHandle, _pScreenTextureImage, _uScreenTextureWidth, _uScreenTextureHeight);

				wcscpy_s(_wcText, wcText);
			}
		}
	}
}

void Application::Render()
{
	// Render Gui
	GEditorManager->RenderGUI();

	// Render Editor list.
	GEditorManager->Render();

	// Render Scene list.
	GSceneManager->Render();

	// Render UI
	GUIManager->Render();
}

void Application::RenderText()
{
	if (!_bChangeEditor)
	{
		GRenderer->RenderSpriteWithTex(GSprite, 10, 10, 1.0f, 1.0f, nullptr, 0.0f, _pTextTextureHandle, AK_TRUE);
	}
	else
	{
		if(!_bPlayingEditor)
		{
			GRenderer->RenderSpriteWithTex(GSprite, 0, 0, 1.0f, 1.0f, nullptr, 0.0f, _pScreenTextureHandle, AK_FALSE);
		}
	}
}

void Application::CalculateFrameRate()
{
	static AkU32 uFrameCount = 0;
	static AkF32 fTimeElapsed = 0.0f;

	uFrameCount++;

	if (GTimer->GetTotalTime() - fTimeElapsed >= 1.0f)
	{
		AkF32 fFps = static_cast<AkF32>(uFrameCount);

		GFps = fFps;

		uFrameCount = 0;
		fTimeElapsed += 1.0f;
	}
}

void Application::ExitGame()
{
	::PostQuitMessage(996);
}

/*
==========================
Input Thread
==========================
*/

void Application::ProcessInputControl()
{
	// TODO
}