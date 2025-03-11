#include "pch.h"
#include "EditorMap.h"
#include "Camera.h"
#include "Terrain.h"

/*
=============
Editor Map
=============
*/

EditorMap::EditorMap()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

EditorMap::~EditorMap()
{
	CleanUp();
}

AkBool EditorMap::Initialize()
{
	Vector3 vCamPos = Vector3(100.0f, 100.0f, -150.0f);
	Vector3 vCamYawPitchRoll = Vector3(-0.6f, 0.5f, 0.0f);
	_pCamera = new Camera(&vCamPos, &vCamYawPitchRoll);
	_pCamera->Mode = CAMERA_MODE::EDITOR;

	// Editor 에서는 Scene에 Obj 를 등록하지 않는다.
	_pTerrainEdit = new TerrainEdit;

	return AK_TRUE;
}

AkBool EditorMap::BeginEditor()
{
	// Scene 또는 다른 Editor 에서 전환된 카메라 위치 방향 조정.
	// TODO!!
	_pCamera->GetTransform()->SetPosition(100.0f, 100.0f, -150.0f);
	_pCamera->GetTransform()->SetRotation(-0.6f, 0.5f, 0.0f);
	_bFPV = AK_FALSE;

	return AK_TRUE;
}

AkBool EditorMap::EndEditor()
{
	return AK_TRUE;
}

void EditorMap::Update()
{
	static AkBool bFirst = AK_TRUE;

	UpdateControl();

	if (bFirst)
	{

	}

	if (_bFPV)
	{
		_pCamera->Update();
	}

	_pTerrainEdit->Update();
}

void EditorMap::FinalUpdate()
{
	_pCamera->UpdateEditor();
	_pTerrainEdit->UpdateEditor();

	ImGui::Begin("Map Editor");
	ImGui::Checkbox("FPV", &_bFPV);
	if (ImGui::Button("Load Height"))
		_pTerrainEdit->LoadHeightMap(L"Test");
	if (ImGui::Button("Save Height"))
		_pTerrainEdit->SaveHeightMap(L"Test");
	if (ImGui::Button("Load Splatting"))
		_pTerrainEdit->LoadSplatingTexture(L"Splating");
	if (ImGui::Button("Save Splatting"))
		_pTerrainEdit->SaveSplatingTexture(L"Splating");

	ImGui::End();
}

void EditorMap::Render()
{
	_pTerrainEdit->Render();
}

void EditorMap::RenderShadow()
{
}

void EditorMap::Load()
{

}

void EditorMap::Save()
{

}

void EditorMap::CleanUp()
{
	if (_pTerrainEdit)
	{
		delete _pTerrainEdit;
		_pTerrainEdit = nullptr;
	}
	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}
}

void EditorMap::UpdateControl()
{
	if (KEY_DOWN(KEY_INPUT_F))
	{
		_bFPV = !_bFPV;
	}
}

