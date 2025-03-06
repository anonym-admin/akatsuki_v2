#include "pch.h"
#include "Editor.h"
#include "Camera.h"
#include "Application.h"
#include "SceneManager.h"
#include "Scene.h"
#include "LandScape.h"

UEditor::UEditor()
{
}

UEditor::~UEditor()
{
	CleanUp();
}

AkBool UEditor::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	_pRenderer = _pApp->GetRenderer();

	return AK_TRUE;
}

void UEditor::BeginEditor()
{
}

void UEditor::EndEditor()
{

}

void UEditor::Update(const AkF32 fDeltaTime)
{
}

void UEditor::Render()
{
}

void UEditor::CleanUp()
{
}

void UEditor::CreateEditorCamera()
{
	_pEditorCam = new UEditorCamera;
	if (!_pEditorCam->Initialize(_pApp))
	{
		__debugbreak();
	}
}

void UEditor::DestroyEditorCamera()
{
	if (_pEditorCam)
	{
		delete _pEditorCam;
		_pEditorCam = nullptr;
	}
}
