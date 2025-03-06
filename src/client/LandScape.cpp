#include "pch.h"
#include "LandScape.h"
#include "Application.h"
#include "GeometryGenerator.h"
#include "Collider.h"
#include "SceneManager.h"
#include "SceneInGame.h"
#include "Actor.h"
#include "DirectXMesh.h"

#include <fstream>

/*
==============
LandScape
==============
*/

ULandScape::ULandScape()
{
}

ULandScape::~ULandScape()
{
	CleanUp();
}

AkBool ULandScape::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	_pRenderer = pApp->GetRenderer();

	// Read height map.
	AkU8* pImage = nullptr;
	AkU32 uWidth = 0;
	AkU32 uHeight = 0;
	if (ReadBitmapFile(L"../../assets/landscape/heightmap01.bmp", &pImage, &uWidth, &uHeight))
	{
	}

	// Make grid geometry.
	AkU32 uMeshDataNum = 0;
	Vector2 vTexScale = Vector2(32.0f);
	MeshData_t* pGrid = UGeometryGenerator::MakeGrid(&uMeshDataNum, 100.0f, uWidth - 1, uHeight - 1, &vTexScale);

	AkU32 k = 0;
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < pGrid[i].uVerticeNum; j++)
		{
			pGrid[i].pVertices[j].vPosition = Vector3::Transform(pGrid[i].pVertices[j].vPosition, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
			pGrid[i].pVertices[j].vPosition.y = pImage[k];
			pGrid[i].pVertices[j].vPosition.y /= 15.0f;

			k += 3;
		}
	}

	if (pImage)
	{
		delete[] pImage;
		pImage = nullptr;
	}

	// Create basic mesh obj.
	_pMeshObj = _pRenderer->CreateBasicMeshObject();
	wcscpy_s(pGrid->wcAlbedoTextureFilename, L"../../assets/landscape/dirt01.dds");
	if (_pMeshObj->CreateMeshBuffers(pGrid, uMeshDataNum))
	{
		// _pMeshObj->EnableWireFrame();
		UGeometryGenerator::DestroyGeometry(pGrid, uMeshDataNum);
	}
	else
	{
		__debugbreak();  
		return AK_FALSE;
	}

	return AK_TRUE;
}

AkBool ULandScape::Initialize(UApplication* pApp, const wchar_t* wcModelFilename, const wchar_t* wcTextureFilename)
{
	_pApp = pApp;

	_pRenderer = pApp->GetRenderer();

	_pMeshObj = _pRenderer->CreateBasicMeshObject();

	_pMeshData = UGeometryGenerator::ReadFromFile(pApp, &_uMeshDataNum, L"../../assets/landscape/", wcModelFilename);

	for (AkU32 i = 0; i < _uMeshDataNum; i++)
		wcscpy_s(_pMeshData[i].wcAlbedoTextureFilename, wcTextureFilename);

	_pMeshObj->CreateMeshBuffers(_pMeshData, _uMeshDataNum);

	_pMeshObj->EnableWireFrame();

	return AK_TRUE;
}

AkBool ULandScape::Initialize(UApplication* pApp, const wchar_t* wcRawSetUpFilename)
{
	_pApp = pApp;

	_pRenderer = pApp->GetRenderer();

	if (!LoadSetupFile(wcRawSetUpFilename))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (!LoadRawHeightMap())
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Make grid geometry.
	AkU32 uMeshDataNum = 0;
	Vector2 vTexScale = Vector2(1.0f);
	MeshData_t* pGrid = UGeometryGenerator::MakeGrid(&uMeshDataNum, 1026.0f, _iLandScapeWidth - 1, _iLandScapeHeight - 1, &vTexScale);

	AkU32 k = 0;
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < pGrid[i].uVerticeNum; j++)
		{
			pGrid[i].pVertices[j].vPosition = Vector3::Transform(pGrid[i].pVertices[j].vPosition, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
			pGrid[i].pVertices[j].vPosition.y = _pHeightMap[k];

			k++;
		}
	}

	// Compute Noramls.
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		DirectX::XMFLOAT3* pPositions = new DirectX::XMFLOAT3[pGrid[i].uVerticeNum];
		DirectX::XMFLOAT3* pNormals = new DirectX::XMFLOAT3[pGrid[i].uVerticeNum];
		DirectX::XMFLOAT2* pTexCoords = new DirectX::XMFLOAT2[pGrid[i].uVerticeNum];

		for (AkU32 j = 0; j < pGrid[i].uVerticeNum; j++)
		{
			Vertex_t tVertex = {};

			if (pGrid[i].pVertices)
			{
				tVertex = pGrid[i].pVertices[j];
				pPositions[j] = tVertex.vPosition;
				pTexCoords[j] = tVertex.vTexCoord;
			}
		}

		DirectX::ComputeNormals(pGrid[i].pIndices, pGrid[i].uIndicesNum / 3, pPositions, pGrid[i].uVerticeNum, DirectX::CNORM_DEFAULT, pNormals);

		if (pGrid->pVertices)
		{
			for (AkU32 j = 0; j < pGrid[i].uVerticeNum; j++)
			{
				pGrid[i].pVertices[j].vNormalModel = pNormals[j];
			}
		}

		delete[] pTexCoords;
		delete[] pNormals;
		delete[] pPositions;
	}
	
	// Compute Tangents.
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		DirectX::XMFLOAT3* pPositions = new DirectX::XMFLOAT3[pGrid[i].uVerticeNum];
		DirectX::XMFLOAT3* pNormals = new DirectX::XMFLOAT3[pGrid[i].uVerticeNum];
		DirectX::XMFLOAT2* pTexCoords = new DirectX::XMFLOAT2[pGrid[i].uVerticeNum];
		DirectX::XMFLOAT3* pTangents = new DirectX::XMFLOAT3[pGrid[i].uVerticeNum];
		DirectX::XMFLOAT3* pBiTangents = new DirectX::XMFLOAT3[pGrid[i].uVerticeNum];

		for (AkU32 j = 0; j < pGrid[i].uVerticeNum; j++)
		{
			Vertex_t tVertex = {};

			if (pGrid[i].pVertices)
			{
				tVertex = pGrid[i].pVertices[j];
				pPositions[j] = tVertex.vPosition;
				pNormals[j] = tVertex.vNormalModel;
				pTexCoords[j] = tVertex.vTexCoord;
			}
		}

		DirectX::ComputeTangentFrame(pGrid[i].pIndices, pGrid[i].uIndicesNum / 3, pPositions, pNormals, pTexCoords, pGrid[i].uVerticeNum, pTangents, pBiTangents);

		if (pGrid->pVertices)
		{
			for (AkU32 j = 0; j < pGrid[i].uVerticeNum; j++)
			{
				pGrid[i].pVertices[j].vTangentModel = pTangents[j];
			}
		}

		delete[] pBiTangents;
		delete[] pTangents;
		delete[] pTexCoords;
		delete[] pNormals;
		delete[] pPositions;
	}


	// Vertex data 재구축.
	_pMeshData = new MeshData_t;
	_uMeshDataNum = 1;

	Vertex_t* pVertices = new Vertex_t[pGrid->uIndicesNum];
	AkU32* pIndices = new AkU32[pGrid->uIndicesNum];

	AkU32 uVerticesIndex = 0;
	AkU32 uIndicesIndex = 0;
	for (AkU32 i = 0; i < pGrid->uIndicesNum; i += 3)
	{
		AkU32 i0 = pGrid->pIndices[i];
		AkU32 i1 = pGrid->pIndices[i + 1];
		AkU32 i2 = pGrid->pIndices[i + 2];

		pVertices[uVerticesIndex++] = pGrid->pVertices[i0];
		pVertices[uVerticesIndex++] = pGrid->pVertices[i1];
		pVertices[uVerticesIndex++] = pGrid->pVertices[i2];
					
		pIndices[uIndicesIndex++] = i;
		pIndices[uIndicesIndex++] = i + 1;
		pIndices[uIndicesIndex++] = i + 2;
	}

	_pMeshData->pVertices = pVertices;
	_pMeshData->pIndices = pIndices;
	_pMeshData->uVerticeNum = pGrid->uIndicesNum;
	_pMeshData->uIndicesNum = pGrid->uIndicesNum;
	wcscpy_s(_pMeshData->wcAlbedoTextureFilename, L"../../assets/landscape/colormap.dds");

	// Mesh OBject 생성.
	Vector3 vAlbedo = Vector3(1.0f);
	AkF32 fMetallic = 0.0f;
	AkF32 fRoughness = 1.0f;
	Vector3 vEmissive = Vector3(0.0f, 0.0f, 0.0f);
	_pMeshObj = _pRenderer->CreateBasicMeshObject();
	_pMeshObj->CreateMeshBuffers(_pMeshData, _uMeshDataNum);
	_pMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmissive);

	if (pGrid)
	{
		UGeometryGenerator::DestroyGeometry(pGrid, uMeshDataNum);
	}

	return AK_TRUE;
}

void ULandScape::Update(const AkF32 fDeltaTime)
{
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; i++)
	{
		if (GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_MAP == (GAME_OBJECT_GROUP_TYPE)i)
		{
			continue;
		}

		UpdateGroupObject((GAME_OBJECT_GROUP_TYPE)i);
	}
}

void ULandScape::Render()
{
	_pRenderer->RenderBasicMeshObject(_pMeshObj, &_mWorld);
}

MeshData_t* ULandScape::GetMeshData(AkU32* pMeshDataNum)
{
	*pMeshDataNum = _uMeshDataNum;

	return _pMeshData;
}

void ULandScape::CleanUp()
{
	if (_pHeightMap)
	{
		delete[] _pHeightMap;
		_pHeightMap = nullptr;
	}
	if (_cColorMapFilename)
	{
		delete[] _cColorMapFilename;
	}
	if (_pMeshData)
	{
		UGeometryGenerator::DestroyGeometry(_pMeshData, _uMeshDataNum);
		_pMeshData = nullptr;
	}
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}
}

AkBool ULandScape::LoadSetupFile(const wchar_t* wcRawSetUpFilename)
{
	using namespace std;

	// 지형 파일 이름과 색상 맵 파일 이름을 포함 할 문자열을 초기화합니다.
	AkI32 stringLength = 256;

	_cTerrainFilename = new char[stringLength];
	if (!_cTerrainFilename)
	{
		return false;
	}

	_cColorMapFilename = new char[stringLength];
	if (!_cColorMapFilename)
	{
		return false;
	}

	// 설치 파일을 엽니다. 파일을 열 수 없으면 종료합니다.
	ifstream fin;
	fin.open(wcRawSetUpFilename);
	if (fin.fail())
	{
		return false;
	}

	// 지형 파일 이름까지 읽습니다.
	char input = 0;
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// 지형 파일 이름을 읽습니다.
	fin >> _cTerrainFilename;

	// 지형 높이 값을 읽습니다.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// 지형 높이를 읽습니다.
	fin >> _iLandScapeHeight;

	// 지형 너비 값을 읽습니다.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// 지형 폭을 읽습니다.
	fin >> _iLandScapeWidth;

	// 지형 높이 배율 값을 읽습니다.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// 지형 높이 스케일링을 읽습니다.
	fin >> _fHeightScale;

	// 컬러 맵 파일 이름을 읽습니다.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// 컬러 맵 파일 이름을 읽습니다.
	fin >> _cColorMapFilename;

	// 설정 파일을 닫습니다.
	fin.close();

	return AK_TRUE;
}

AkBool ULandScape::LoadRawHeightMap()
{
	// 높이 맵 데이터를 보관할 플로트 배열을 만듭니다.
	_pHeightMap = new AkF32[_iLandScapeWidth * _iLandScapeHeight];
	if (!_pHeightMap)
	{
		return false;
	}

	// 바이너리로 읽을 수 있도록 16 비트 원시 높이 맵 파일을 엽니다.
	FILE* filePtr = nullptr;
	if (fopen_s(&filePtr, _cTerrainFilename, "rb") != 0)
	{
		return false;
	}

	// 원시 이미지 데이터의 크기를 계산합니다.
	int imageSize = _iLandScapeHeight * _iLandScapeWidth;

	// 원시 이미지 데이터에 메모리를 할당합니다.
	unsigned short* rawImage = new unsigned short[imageSize];
	if (!rawImage)
	{
		return false;
	}

	// 원시 이미지 데이터를 읽습니다.
	if (fread(rawImage, sizeof(unsigned short), imageSize, filePtr) != imageSize)
	{
		return false;
	}

	// 파일을 닫습니다.
	if (fclose(filePtr) != 0)
	{
		return false;
	}

	// 이미지 데이터를 높이 맵 배열에 복사합니다.
	for (int j = 0; j < _iLandScapeHeight; j++)
	{
		for (int i = 0; i < _iLandScapeWidth; i++)
		{
			int index = (_iLandScapeWidth * j) + i;

			// 높이 맵 배열에이 지점의 높이를 저장합니다.
			_pHeightMap[index] = (float)rawImage[index] / _fHeightScale;
		}
	}

	// 비트 맵 이미지 데이터를 해제합니다.
	delete[] rawImage;
	rawImage = 0;

	// 이제 읽은 지형 파일 이름을 해제합니다.
	delete[] _cTerrainFilename;
	_cTerrainFilename = 0;

	return true;
}

void ULandScape::UpdateGroupObject(GAME_OBJECT_GROUP_TYPE eType)
{
	USceneManager* pSceneManger = _pApp->GetSceneManager();
	USceneInGame* pSceneInGame = (USceneInGame*)pSceneManger->GetCurrentScene();

	GameObjContainer_t* pGameObjContainer = pSceneInGame->GetGroupObject(eType);
	if (!pGameObjContainer)
	{
		return;
	}

	List_t* pCur = pGameObjContainer->pGameObjHead;
	while (pCur != nullptr)
	{
		UActor* pPlayer = (UActor*)pCur->pData;

		Vector3 vPlayerPos = pPlayer->GetPosition();
		Vector3 vOffset = Vector3((AkF32)_iLandScapeWidth / 2.0f, 0.0f, (AkF32)_iLandScapeHeight / 2.0f);
		Vector3 vHeightMapPos = vPlayerPos + vOffset;

		AkI32 iMinX = (AkI32)floor(vHeightMapPos.x);
		AkI32 iMaxX = (AkI32)ceil(vHeightMapPos.x);
		AkI32 iMinZ = (AkI32)floor(vHeightMapPos.z);
		AkI32 iMaxZ = (AkI32)ceil(vHeightMapPos.z);

		AkI32 iHeightIndex0 = iMinX + iMinZ * _iLandScapeWidth;
		AkI32 iHeightIndex1 = iMinX + iMaxZ * _iLandScapeWidth;
		AkI32 iHeightIndex2 = iMaxX + iMaxZ * _iLandScapeWidth;
		AkI32 iHeightIndex3 = iMaxX + iMinZ * _iLandScapeWidth;

		AkF32 fH0 = _pHeightMap[iHeightIndex0];
		AkF32 fH1 = _pHeightMap[iHeightIndex1];
		AkF32 fH2 = _pHeightMap[iHeightIndex2];
		AkF32 fH3 = _pHeightMap[iHeightIndex3];

		Vector3 vVert0 = Vector3((AkF32)iMinX, fH0, (AkF32)iMinZ);
		Vector3 vVert1 = Vector3((AkF32)iMinX, fH1, (AkF32)iMaxZ);
		Vector3 vVert2 = Vector3((AkF32)iMaxX, fH2, (AkF32)iMaxZ);
		Vector3 vVert3 = Vector3((AkF32)iMaxX, fH3, (AkF32)iMinZ);

		Vector3 vRayPos = Vector3(vHeightMapPos.x, 1000.0f, vHeightMapPos.z);
		Vector3 vRayDir = Vector3(0.0f, -1.0f, 0.0f);
		DirectX::SimpleMath::Ray tRay(vRayPos, vRayDir);

		AkF32 fDist0 = 0.0f;
		AkF32 fDist1 = 0.0f;
		Vector3 vHitPos0 = Vector3(0.0f);
		Vector3 vHitPos1 = Vector3(0.0f);
		AkBool bIntersectTri0 = tRay.Intersects(vVert0, vVert1, vVert2, fDist0);
		AkBool bIntersectTri1 = tRay.Intersects(vVert0, vVert2, vVert3, fDist1);

		if (bIntersectTri0 || bIntersectTri1)
		{
			if (bIntersectTri0)
			{
				vHitPos0 = vRayPos + vRayDir * fDist0;

				pPlayer->SetPosition(vPlayerPos.x, vHitPos0.y + 1.0f, vPlayerPos.z);
			}
			else
			{
				vHitPos1 = vRayPos + vRayDir * fDist1;

				pPlayer->SetPosition(vPlayerPos.x, vHitPos1.y + 1.0f, vPlayerPos.z);
			}
		}
		else
		{
			AkU32 i = 0;
		}

		pCur = pCur->pNext;
	}
}
