#pragma once

/*
==============
LandScape
==============
*/

class UApplication;

class ULandScape
{
public:
	ULandScape();
	~ULandScape();

	AkBool Initialize(UApplication* pApp);
	AkBool Initialize(UApplication* pApp, const wchar_t* wcModelFilename, const wchar_t* wcTextureFilename);
	AkBool Initialize(UApplication* pApp, const wchar_t* wcRawSetUpFilename);
	void Update(const AkF32 fDeltaTime);
	void Render();

	MeshData_t* GetMeshData(AkU32* pMeshDataNum);
	AkF32* GetHeightMap() { return _pHeightMap; }
	AkI32 GetLandScapeWidth() { return _iLandScapeWidth; }
	AkI32 GetLandScapeHeight() { return _iLandScapeHeight; }

private:
	void CleanUp();

	AkBool LoadSetupFile(const wchar_t* wcRawSetUpFilename);
	AkBool LoadRawHeightMap();
	void UpdateGroupObject(GAME_OBJECT_GROUP_TYPE eType);

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;
	IMeshObject* _pMeshObj = nullptr;
	Matrix _mWorld = Matrix();

	MeshData_t* _pMeshData = nullptr;
	AkU32 _uMeshDataNum = 0;

	char* _cTerrainFilename = nullptr;
	char* _cColorMapFilename = nullptr;
	AkI32 _iLandScapeHeight = 0;
	AkI32 _iLandScapeWidth = 0;
	AkF32 _fHeightScale = 0.0f;

	AkF32* _pHeightMap = nullptr;
};

