#pragma once

#include "Editor.h"

/*
=============
Editor Map
=============
*/

class TerrainEdit;
class Billboard;
class Light;
class Ocean;
class Cloud;
class Grass;

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
	virtual void RenderShadowMaps() override;
	virtual void Render() override;

protected:
	virtual void Load(const std::wstring& wcFilePath) override; // Load Scene.
	virtual void Save(const std::wstring& wcFilePath) override; // Save Scene.

private:
	void CleanUp();

	Billboard* CreateBillboards(const std::wstring& wcFilePath, VertexSize_t* pVertices, AkU32 uNum);
	Ocean* CreateOcean();
	Cloud* CreateCloud();
	Grass* CreateGrass();
	Light* CreateLight();

	void ImportMap(const std::wstring& wcFilePath);
	void ImportActor(const std::wstring& wcFilePath);
	void ExportMap(const std::wstring& wcFilePath);

	void UpdateControl();
	void UpdateFileDialog();
	void UpdateGizmo();

	void UpdateObject();
	void UpdateObjectDeleteState();

private:
	Camera* _pCamera = nullptr;
	TerrainEdit* _pTerrainEdit = nullptr;

	// First Person View Flag.
	AkBool _bFPV = AK_TRUE;
	AkBool _bLoad = AK_FALSE;
	AkBool _bSave = AK_FALSE;

	AkI32 _iSelectMode = 0; // 0: Height 1: Splating 2: Texture 3: Map
	AkI32 _iTextureType = 0; // 0: Albedo ~ 5 : AO 6: Second 7: Third
	AkI32 _iBillboardType = -1; // 0: Tree 1: Grass

	// 해당 클래스에서 임시로 쓸 수 있는 문자열
	std::wstring _wcFileName = L"";
	std::wstring _wcFileNameExcExt = L"";
	std::wstring _wcFilePath = L"";

	// Save For Map Data.
	std::wstring _wcMapFileName = L"";

	// 01. Terrain Texture name
	std::wstring _wcAlbedoFilename = L"";
	std::wstring _wcNormalFilename = L"";
	std::wstring _wcEmissiveFilename = L"";
	std::wstring _wcMetallicFilename = L"";
	std::wstring _wcRoughnessFilename = L"";
	std::wstring _wcAOFilename = L"";
	std::wstring _wcSecondFilename = L"";
	std::wstring _wcThirdFilename = L"";

	// 02. Height Map
	std::wstring _wcHeightFilename = L"";

	// 03. Splatting Alpha Map
	std::array<std::wstring, 2> _wcAlphaFilenames = {};

	// 04. Game Objs
	std::unordered_map<std::wstring, std::vector<std::pair<Actor*, AkBool>>> _mapGameObj = {}; // Pair bool : dead flag

	// 05. Tree Billboard
	std::unordered_map<std::wstring, Billboard*> _mapBillboard = {};
	std::unordered_map<std::wstring, std::array<VertexSize_t, 20>> _mapVertices = {};

	// Light
	std::vector<Light*> _vecLights = {};
};

