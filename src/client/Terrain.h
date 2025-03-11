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

	Terrain();
	~Terrain();

	AkBool Initialize();
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
	void LoadHeightMap(const wchar_t* wcHeightFile);

private:
	AkU32 _uWidth = 100;
	AkU32 _uHeight = 100;

	IMeshObject* _pMeshObj = nullptr;

	MeshData_t* _pGrid = nullptr;
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
	~TerrainEdit();

	AkBool Initialize();
	void Update();
	void UpdateEditor();
	void Render();

	void Load(const wchar_t* wcHeightFile);
	void Save(const wchar_t* wcHeightFile);

private:
	void CleanUp();

	void CreateMeshData();

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

	TerrainVertex_t* _pVertices = nullptr;
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
	AkF32 _fHeightMin = 0.0f;

	AkF32 _fPaintScale = 5.0f;
	AkI32 _iSelectedTexture = 0;

	AkI32 _iEditType = 0; // 0 : Paint , 1 : Height
};

