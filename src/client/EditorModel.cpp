#include "pch.h"
#include "EditorModel.h"
#include "Scene.h"
#include "Actor.h"
#include "SkinnedModel.h"
#include "Animation.h"
#include "Camera.h"
#include "Transform.h"

#include <string>

EditorModel::EditorModel()
{

}

EditorModel::~EditorModel()
{
	EndEditor();
}

AkBool EditorModel::BeginEditor()
{
	return AK_TRUE;
}

AkBool EditorModel::EndEditor()
{
	return AK_TRUE;
}

void EditorModel::Update()
{

}

void EditorModel::FinalUpdate()
{
	ImGui::Begin("Model Editor");
	ImGui::End();
}

void EditorModel::Render()
{

}

void EditorModel::RenderShadow()
{

}

void EditorModel::Load(const std::wstring& wcFilePath)
{
}

void EditorModel::Save(const std::wstring& wcFilePath)
{
}

