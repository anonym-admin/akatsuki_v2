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
	virtual void RenderGUI() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

protected:
	virtual void Load(const std::wstring& wcFilePath) override;
	virtual void Save(const std::wstring& wcFilePath) override;

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

	AkI32 _iSelectMode = 0; // 0: Height 1: Splating 2: Texture 3: Map
	AkI32 _iTextureType = 0; // 0: Albedo 1: Second 2: Third

	// 해당 클래스에서 임시로 쓸 수 있는 문자열
	std::wstring _wcFileName = L"";
	std::wstring _wcFileNameExcExt = L"";
	std::wstring _wcFilePath = L"";

	// Save For Map Data.

	// 01. Texture name
	std::wstring _wcAlbedoFilename = L"";
	std::wstring _wcSecondFilename = L"";
	std::wstring _wcThirdFilename = L"";

	// 02. Height Map
	std::wstring _wcHeightFilename = L"";

	// 03. Splatting Alpha Map
	std::array<std::wstring, 2> _wcAlphaFilenames = {};
};

