#include "pch.h"
#include "EditorMap.h"

EditorMap::EditorMap()
{

}

EditorMap::~EditorMap()
{
	EndEditor();
}

AkBool EditorMap::BeginEditor()
{
	return AK_TRUE;
}

AkBool EditorMap::EndEditor()
{
	return AK_TRUE;
}

void EditorMap::Update()
{

}

void EditorMap::FinalUpdate()
{
	//ImGui::Begin("Map Editor");
	//ImGui::End();
}

void EditorMap::Render()
{

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
