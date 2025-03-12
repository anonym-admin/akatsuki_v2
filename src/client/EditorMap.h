#pragma once

#include "Editor.h"

/*
=============
Editor Map
=============
*/

class TerrainEdit;

class EditorMap : public Editor
{
public:
	EditorMap();
	~EditorMap();

	AkBool Initialize();
	virtual AkBool BeginEditor() override;
	virtual AkBool EndEditor() override;
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

protected:
	virtual void Load() override;
	virtual void Save() override;

private:
	void CleanUp();

	void UpdateControl();
	void UpdateFileDialog();

private:
	Camera* _pCamera = nullptr;
	TerrainEdit* _pTerrainEdit = nullptr;

	// First Person View Flag.
	AkBool _bFPV = AK_TRUE;
	AkBool _bLoad = AK_FALSE;
	AkBool _bSave = AK_FALSE;

	AkI32 _iSelectMode = 0; // 0: Height 1: Splating 2: Texture
	AkI32 _iTextureType = 0; // 0: Albedo 1: Second 2: Third

	std::wstring _wcFileName = L"";
	std::wstring _wcFileNameExcExt = L"";
	std::wstring _wcFilePath = L"";
};

