#pragma once

#include "Actor.h"

/*
=========
Terrain
=========
*/

class Terrain : public Actor
{
public:
	static AkBool DRAW_WIRE;

	Terrain(const wchar_t* wcSetUpFile);
	~Terrain();

	AkBool Initialize(const wchar_t* wcSetUpFile);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void RenderShadow() override;
	virtual void Render() override;

	virtual void OnCollision(class Collider* pOther) override;
	virtual void OnCollisionEnter(class  Collider* pOther) override;
	virtual void OnCollisionExit(class Collider* pOther) override;

private:
	void CleanUp();

	void CreateMeshData();
	void DestroyMeshData();

	void ComputeNormals();
	void ComputeTangents();

	void LoadSetUpFile(const wchar_t* wcSetUpFile);
	void LoadHeightMap(const wchar_t* wcHeightFile);
	void LoadSplatingTexture(const wchar_t* wcAlphaFile, AkI32* pOutSplattingID);

	void UpdateHeight(Actor* pActor);

private:
	AkU32 _uWidth = 100;
	AkU32 _uHeight = 100;

	ITerrain* _pTerrain = nullptr;
	VertexNormalTexcoordTangentAlpha_t* _pVertices = nullptr;
	AkU32* _pIndices = nullptr;
	AkU32 _uVerticeNum = 0;
	AkU32 _uIndiceNum = 0;
	Vector2 _vTexScale = Vector2(16.0f);

	AkU8* _pHeightMapImg = nullptr;

	const AkF32 MAX_HEIGHT = 30.0f;

	wchar_t wcSplatingFilenames[2][_MAX_PATH] = {};
	wchar_t wcAlphaFilenames[2][_MAX_PATH] = {};
	wchar_t wcHeightMapFilename[_MAX_PATH] = {};
	wchar_t wcAlbedoFilename[_MAX_PATH] = {};
	wchar_t wcNormalFilename[_MAX_PATH] = {};
	wchar_t wcEmissiveFilename[_MAX_PATH] = {};
	wchar_t wcMetallicFilename[_MAX_PATH] = {};
	wchar_t wcRoughnewwFilename[_MAX_PATH] = {};
	wchar_t wcAOFilename[_MAX_PATH] = {};
};

/*
=================
Terrain Edit
=================
*/

class TerrainEdit
{
	struct Brush_t // RendererType.h
	{
		AkI32 iType = 0;
		Vector3 vPos = Vector3(0.0f);
		AkF32 fRange = 10.0f;
		Vector3 vColor = Vector3(0.5f, 0.0f, 0.0f);
	} _tBrush;

public:
	TerrainEdit();
	TerrainEdit(const wchar_t* wcSetUpFile);
	~TerrainEdit();

	AkBool Initialize();
	AkBool Initialize(const wchar_t* wcSetUpFile);
	void Update();
	void UpdateEditor();
	void Render();

	void LoadSetUpFile(const wchar_t* wcSetUpFile);
	void LoadHeightMap(const wchar_t* wcHeightFile);
	void SaveHeightMap(const wchar_t* wcHeightFile);
	void LoadSplatingTexture(const wchar_t* wcAlphaFile, AkI32* pOutSelectedID);
	void SaveSplatingTexture(const wchar_t* wcAlphaFile, AkI32* pOutSelectedID);

	void SetTextures(const wchar_t* wcAlbedoFilePath, const wchar_t* wcNormalFilePath, const wchar_t* wcEmissiveFilePath, const wchar_t* wcMetallicFilePath, const wchar_t* wcRoughnewssFilePath, const wchar_t* wcAOFilePath, const wchar_t* wcSecondTexFilePath, const wchar_t* wcThirdTexFilePath);

private:
	void CleanUp();

	void CreateMeshData();
	void CreateRenderObject();
	void DestroyMeshData();
	void DestroyRenderObject();

	void ComputeHeight();
	void ComputeNormals();
	void ComputeTangents();

	void PaintBrush();

	void UpdateMousePicking();

private:
	AkU32 _uWidth = 100;
	AkU32 _uHeight = 100;
	AkBool _bDrawWire = AK_FALSE;
	AkBool _bDrawBrush = AK_FALSE;
	AkBool _bPicked = AK_FALSE;
	AkBool _bPositive = AK_TRUE;

	VertexNormalTexcoordTangentAlpha_t* _pVertices = nullptr;
	AkU32* _pIndices = nullptr;
	AkU32 _uVerticeNum = 0;
	AkU32 _uIndiceNum = 0;

	ITerrain* _pTerrain = nullptr;
	Matrix _mWorldRow = Matrix();

	void* _pDVHandle = nullptr;

	Vector3 _vPickPos = Vector3(0.0f);
	AkF32 _fPickDist = 0.0f;
	AkF32 _fMoveRatio = 0.0f;
	AkF32 _fHeightScale = 50.0f;

	AkF32 _fPaintScale = 5.0f;
	AkI32 _iSelectedTexture = 0;

	AkI32 _iEditType = 0; // 0 : Paint , 1 : Height

	const AkF32 MAX_ALPHA = 1.0f;
	const AkF32 MAX_HEIGHT = 30.0f;
	AkF32 MIN_HEIGHT = 0.0f;

	AkU8* _pHeightMapImg = nullptr;

	Vector2 _vTexScale = Vector2(1.0f);

	std::wstring _wcAlbedoFilePath = L"";
	std::wstring _wcNormalFilePath = L"";
	std::wstring _wcEmissiveilePath = L"";
	std::wstring _wcMetallicFilePath = L"";
	std::wstring _wcRoughnessFilePath = L"";
	std::wstring _wcAOFilePath = L"";
	std::wstring _wcSecondTexFilePath = L"";
	std::wstring _wcThirdTexFilePath = L"";
	
	std::wstring _wcHeightMapFileName = L"";
	std::wstring _wcAlphaFilenames[2] = {};
};

