#include "pch.h"
#include "Terrain.h"

#pragma warning(disable : 4244)

/*
=========
Terrain
=========
*/

AkBool Terrain::DRAW_WIRE;

Terrain::Terrain()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

Terrain::~Terrain()
{
	CleanUp();
}

AkBool Terrain::Initialize()
{
	// Load Resource.
	LoadHeightMap(L"Test");
	for (AkU32 i = 0; i < _countof(wcSplatingFilenames); i++)
	{
		LoadSplatingTexture(L"Splating", i);
	}

	// Create Render obj.
	_pTerrain = GRenderer->CreateTerrain();
	_pTerrain->CreateStaticMeshBuffers(_pVertices, _uVerticeNum, _pIndices, _uIndiceNum);
	_pTerrain->SetTextures(wcSplatingFilenames[0], wcSplatingFilenames[1], L"../../assets/map/soil.dds");

	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissvie = Vector3(0.0f);
	_pTerrain->UpdateMaterialBuffers(&vAlbedo, 0.0f, 1.0f, &vEmissvie);

	// Create transform.
	_pTransform = CreateTransform();

	// Create Collider.
	_pCollider = CreateSquareCollider(); // 더미 콜라이더를 생성 => 충돌 매니저의 기존 로직을 유지하기 위해서

	return AK_TRUE;
}

void Terrain::Update()
{
	DRAW_WIRE ? _pTerrain->EnableWireFrame() : _pTerrain->DisableWireFrame();
}

void Terrain::FinalUpdate()
{
	_pTransform->Update();
}

void Terrain::RenderShadow()
{
}

void Terrain::Render()
{
	GRenderer->RenderTerrain(_pTerrain, &_pTransform->GetWorldTransform(), nullptr);
}

void Terrain::OnCollision(Collider* pOther)
{
	Actor* pOtherActor = pOther->GetOwner();
	if (!wcscmp(pOtherActor->Name, L"Swat"))
	{
		UpdateHeight(pOtherActor);
	}
}

void Terrain::OnCollisionEnter(Collider* pOther)
{
	Actor* pOtherActor = pOther->GetOwner();
	if (!wcscmp(pOtherActor->Name, L"Swat"))
	{
		UpdateHeight(pOtherActor);
	}
}

void Terrain::OnCollisionExit(Collider* pOther)
{
}

void Terrain::CleanUp()
{
	if (_pTerrain)
	{
		_pTerrain->Release();
		_pTerrain = nullptr;
	}

	DestroyMeshData();
}

void Terrain::CreateMeshData()
{
	DestroyMeshData();

	Vector4* pPixels = nullptr;
	if (_pHeightMapImg)
	{
		AkU32 uSize = _uWidth * _uHeight;
		pPixels = new Vector4[uSize];
		ImageToPixel(_pHeightMapImg, pPixels, uSize * 4);
	}

	AkU32 uMeshDataNum = 0;
	MeshData_t* pGrid = GeometryGenerator::MakeGrid(&uMeshDataNum, (AkF32)_uWidth, (_uWidth - 1), (_uHeight - 1), &_vTexScale); // uMeshDataNum == 1
	for (AkU32 i = 0; i < pGrid->uVerticeNum; i++)
	{
		pGrid->pVertices[i].vPosition = Vector3::Transform(pGrid->pVertices[i].vPosition, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
		pGrid->pVertices[i].vNormalModel = Vector3::Transform(pGrid->pVertices[i].vNormalModel, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
		pGrid->pVertices[i].vTangentModel = Vector3::Transform(pGrid->pVertices[i].vTangentModel, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
	}

	// Change Vertex Type
	_pVertices = new TerrainVertex_t[pGrid->uVerticeNum];
	_pIndices = new AkU32[pGrid->uIndicesNum];
	_uVerticeNum = pGrid->uVerticeNum;
	_uIndiceNum = pGrid->uIndicesNum;

	for (AkU32 i = 0; i < pGrid->uVerticeNum; i++)
	{
		_pVertices[i].vPosition = pGrid->pVertices[i].vPosition;
		if (_pHeightMapImg)
		{
			_pVertices[i].vPosition.y += pPixels[i].x * MAX_HEIGHT;
		}
		_pVertices[i].vNormalModel = pGrid->pVertices[i].vNormalModel;
		_pVertices[i].vTexCoord = pGrid->pVertices[i].vTexCoord;
		_pVertices[i].vTangentModel = pGrid->pVertices[i].vTangentModel;
	}

	memcpy(_pIndices, pGrid->pIndices, sizeof(AkU32) * _uIndiceNum);

	// Delete Origin MeshData.
	if (pGrid)
	{
		GeometryGenerator::DestroyGeometry(pGrid, 1);
		pGrid = nullptr;
	}

	ComputeNormals();
	ComputeTangents();

	if (pPixels)
	{
		delete[] pPixels;
		pPixels = nullptr;
	}
}

void Terrain::DestroyMeshData()
{
	if (_pVertices)
	{
		delete _pVertices;
		_pVertices = nullptr;
	}
	if (_pIndices)
	{
		delete _pIndices;
		_pIndices = nullptr;
	}
}

void Terrain::ComputeNormals()
{
	DirectX::XMFLOAT3* position = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT3* normal = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT2* texCoord = new DirectX::XMFLOAT2[_uVerticeNum];
	DirectX::XMFLOAT3* tangent = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT3* biTangent = new DirectX::XMFLOAT3[_uVerticeNum];

	for (AkU32 i = 0; i < _uVerticeNum; i++)
	{
		TerrainVertex_t v = {};
		if (_pVertices)
		{
			v = _pVertices[i];
			position[i] = v.vPosition;
		}
	}

	DirectX::ComputeNormals(_pIndices, _uIndiceNum / 3, position, _uVerticeNum, DirectX::CNORM_DEFAULT, normal);

	if (_pVertices)
	{
		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			_pVertices[i].vNormalModel = normal[i];
		}
	}

	delete[] position;
	delete[] normal;
	delete[] texCoord;
	delete[] tangent;
	delete[] biTangent;
}

void Terrain::ComputeTangents()
{
	DirectX::XMFLOAT3* position = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT3* normal = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT2* texCoord = new DirectX::XMFLOAT2[_uVerticeNum];
	DirectX::XMFLOAT3* tangent = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT3* biTangent = new DirectX::XMFLOAT3[_uVerticeNum];

	for (AkU32 i = 0; i < _uVerticeNum; i++)
	{
		TerrainVertex_t v = {};
		if (_pVertices)
		{
			v = _pVertices[i];
			position[i] = v.vPosition;
			normal[i] = v.vNormalModel;
			texCoord[i] = v.vTexCoord;
		}
	}

	DirectX::ComputeTangentFrame(_pIndices, _uIndiceNum / 3, position, normal, texCoord, _uVerticeNum, tangent, biTangent);

	if (_pVertices)
	{
		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			_pVertices[i].vTangentModel = tangent[i];
		}
	}

	delete[] position;
	delete[] normal;
	delete[] texCoord;
	delete[] tangent;
	delete[] biTangent;
}

void Terrain::LoadHeightMap(const wchar_t* wcHeightFile)
{
	AkU8* uImg = nullptr;
	AkU32 uWidth = 0;
	AkU32 uHeight = 0;

	wchar_t wcPath[_MAX_PATH] = {};
	wcscat_s(wcPath, MAP_FILE_PATH);
	wcscat_s(wcPath, wcHeightFile);
	wcscat_s(wcPath, L".png");

	ReadImage(wcPath, &uImg, &uWidth, &uHeight);

	_uWidth = uWidth;
	_uHeight = uHeight;

	_pHeightMapImg = uImg;

	CreateMeshData();

	if (uImg)
	{
		delete uImg;
		uImg = nullptr;
	}
}

void Terrain::LoadSplatingTexture(const wchar_t* wcAlphaFile, AkI32 iSplattingID)
{
	AkU8* uImg = nullptr;
	AkU32 uWidth = 0;
	AkU32 uHeight = 0;

	wchar_t wcTemp[2] = {};
	_itow_s(iSplattingID, wcTemp, 10);
	wchar_t wcPath[_MAX_PATH] = {};
	wcscat_s(wcPath, MAP_FILE_PATH);
	wcscat_s(wcPath, wcAlphaFile);
	wcscat_s(wcPath, L"_");
	wcscat_s(wcPath, wcTemp);
	wcscat_s(wcPath, L".png");

	ReadImage(wcPath, &uImg, &uWidth, &uHeight);

	_uWidth = uWidth;
	_uHeight = uHeight;

	Vector4* pPixels = nullptr;
	if (uImg)
	{
		AkU32 uSize = _uWidth * _uHeight;
		pPixels = new Vector4[uSize];
		ImageToPixel(uImg, pPixels, uSize * 4);

		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			switch (iSplattingID)
			{
			case 0:
				_pVertices[i].pAlpha[iSplattingID] = pPixels[i].x;
				break;
			case 1:
				_pVertices[i].pAlpha[iSplattingID] = pPixels[i].y;
				break;
			default:
				__debugbreak();
				break;
			}
		}

		if (pPixels)
		{
			delete[] pPixels;
			pPixels = nullptr;
		}

		delete uImg;
		uImg = nullptr;
	}
}

void Terrain::UpdateHeight(Actor* pActor)
{
	Vector3 vObjPos = pActor->GetTransform()->GetPosition();
	Vector3 vOffset = Vector3((AkF32)_uWidth / 2.0f, 0.0f, (AkF32)_uHeight / 2.0f);
	Vector3 vHeightMapPos = vObjPos + vOffset;

	AkI32 iMinX = (AkI32)floor(vHeightMapPos.x);
	AkI32 iMaxX = (AkI32)ceil(vHeightMapPos.x);
	AkI32 iMinZ = (AkI32)floor(vHeightMapPos.z);
	AkI32 iMaxZ = (AkI32)ceil(vHeightMapPos.z);

	AkI32 iHeightIndex0 = iMinX + iMinZ * _uWidth;
	AkI32 iHeightIndex1 = iMinX + iMaxZ * _uWidth;
	AkI32 iHeightIndex2 = iMaxX + iMaxZ * _uWidth;
	AkI32 iHeightIndex3 = iMaxX + iMinZ * _uWidth;

	AkF32 fH0 = _pVertices[iHeightIndex0].vPosition.y;
	AkF32 fH1 = _pVertices[iHeightIndex1].vPosition.y;
	AkF32 fH2 = _pVertices[iHeightIndex2].vPosition.y;
	AkF32 fH3 = _pVertices[iHeightIndex3].vPosition.y;

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

			Vector3 vPos = Vector3(vObjPos.x, vHitPos0.y + 0.5f, vObjPos.z);
			pActor->GetTransform()->SetPosition(&vPos);
		}
		else
		{
			vHitPos1 = vRayPos + vRayDir * fDist1;

			Vector3 vPos = Vector3(vObjPos.x, vHitPos1.y + 0.5f, vObjPos.z);
			pActor->GetTransform()->SetPosition(&vPos);
		}
	}
	else
	{
		AkU32 i = 0;
	}
}

/*
=================
Terrain Edit
=================
*/

TerrainEdit::TerrainEdit()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

TerrainEdit::~TerrainEdit()
{
	CleanUp();
}

AkBool TerrainEdit::Initialize()
{
	CreateMeshData();
	CreateRenderObject();

	return AK_TRUE;
}

void TerrainEdit::Update()
{
	UpdateMousePicking();

	_bDrawWire ? _pTerrain->EnableWireFrame() : _pTerrain->DisableWireFrame();

	if (_bDrawBrush)
	{
		if (LBTN_HOLD && _bPicked)
		{
			if (_iEditType == 0)
			{
				PaintBrush();
			}
			else if (_iEditType == 1)
			{
				ComputeHeight();
			}
		}
		else
		{
			GRenderer->UpdateDynamicVertices(_pDVHandle, _pVertices);
			ComputeNormals();
			ComputeTangents();
		}
	}
	else
	{
		GRenderer->UpdateDynamicVertices(_pDVHandle, _pVertices);
	}
}

void TerrainEdit::UpdateEditor()
{
	ImGui::Begin("Terrian Edit");
	ImGui::Checkbox("Draw Wire", &_bDrawWire);
	ImGui::Checkbox("Draw Brush", &_bDrawBrush);
	ImGui::Checkbox("Positive", &_bPositive);
	ImGui::SliderFloat("Range Scale", &_tBrush.fRange, 0.0f, 20.0f);
	ImGui::SliderFloat("Height Scale", &_fHeightScale, 0.0f, 50.0f);
	ImGui::SliderFloat("Set Minimize Hegiht", &MIN_HEIGHT, -100.0f, 0.0f);

	const char* pEditType[] = { "Paint", "Height" };
	ImGui::Combo("Edit Type", &_iEditType, pEditType, IM_ARRAYSIZE(pEditType));
	const char* pBrushType[] = { "Sphere", "Square" };
	ImGui::Combo("Brush Type", &_tBrush.iType, pBrushType, IM_ARRAYSIZE(pBrushType));
	if (_iEditType == 0)
	{
		const char* pTex[] = { "Grass", "Stone" };
		ImGui::Combo("Texture", &_iSelectedTexture, pTex, IM_ARRAYSIZE(pTex));
	}
	ImGui::End();
}

void TerrainEdit::Render()
{
	GRenderer->RenderTerrain(_pTerrain, &_mWorldRow, &_tBrush);
}

void TerrainEdit::LoadHeightMap(const wchar_t* wcHeightFile)
{
	AkU8* uImg = nullptr;
	AkU32 uWidth = 0;
	AkU32 uHeight = 0;

	wchar_t wcPath[_MAX_PATH] = {};
	wcscat_s(wcPath, MAP_FILE_PATH);
	wcscat_s(wcPath, wcHeightFile);
	wcscat_s(wcPath, L".png");

	ReadImage(wcPath, &uImg, &uWidth, &uHeight);

	_uWidth = uWidth;
	_uHeight = uHeight;

	_pHeightMapImg = uImg;

	CreateMeshData();

	if (uImg)
	{
		delete uImg;
		uImg = nullptr;
	}
}

void TerrainEdit::SaveHeightMap(const wchar_t* wcHeightFile)
{
	using namespace DirectX;

	AkU32 uSize = _uWidth * _uHeight * 4;
	AkU8* pPixels = new AkU8[uSize];
	for (AkU32 i = 0; i < uSize / 4; i++)
	{
		AkF32 fY = _pVertices[i].vPosition.y;
		AkU8 uHeight = (fY * 255.0f) / MAX_HEIGHT;
		pPixels[(4 * i) + 0] = uHeight;
		pPixels[(4 * i) + 1] = uHeight;
		pPixels[(4 * i) + 2] = uHeight;
		pPixels[(4 * i) + 3] = 255;
	}

	DirectX::Image tImage = {};
	tImage.width = _uWidth;
	tImage.height = _uHeight;
	tImage.pixels = pPixels;
	tImage.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tImage.rowPitch = tImage.width * 4;
	tImage.slicePitch = uSize;

	wchar_t wcPath[_MAX_PATH] = {};
	wcscat_s(wcPath, MAP_FILE_PATH);
	wcscat_s(wcPath, wcHeightFile);
	wcscat_s(wcPath, L".png");

	SaveToWICFile(tImage, WIC_FLAGS_FORCE_RGB, GetWICCodec(WIC_CODEC_PNG), wcPath);

	delete[] pPixels;
	pPixels = nullptr;
}

void TerrainEdit::LoadSplatingTexture(const wchar_t* wcAlphaFile)
{
	AkU8* uImg = nullptr;
	AkU32 uWidth = 0;
	AkU32 uHeight = 0;

	wchar_t wcTemp[2] = {};
	_itow_s(_iSelectedTexture, wcTemp, 10);
	wchar_t wcPath[_MAX_PATH] = {};
	wcscat_s(wcPath, MAP_FILE_PATH);
	wcscat_s(wcPath, wcAlphaFile);
	wcscat_s(wcPath, L"_");
	wcscat_s(wcPath, wcTemp);
	wcscat_s(wcPath, L".png");

	ReadImage(wcPath, &uImg, &uWidth, &uHeight);

	_uWidth = uWidth;
	_uHeight = uHeight;

	Vector4* pPixels = nullptr;
	if (uImg)
	{
		AkU32 uSize = _uWidth * _uHeight;
		pPixels = new Vector4[uSize];
		ImageToPixel(uImg, pPixels, uSize * 4);

		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			switch (_iSelectedTexture)
			{
			case 0:
				_pVertices[i].pAlpha[_iSelectedTexture] = pPixels[i].x;
				break;
			case 1:
				_pVertices[i].pAlpha[_iSelectedTexture] = pPixels[i].y;
				break;
			default:
				__debugbreak();
				break;
			}
		}

		if(pPixels)
		{
			delete[] pPixels;
			pPixels = nullptr;
		}

		delete uImg;
		uImg = nullptr;
	}
}

void TerrainEdit::SaveSplatingTexture(const wchar_t* wcAlphaFile)
{
	using namespace DirectX;

	AkU32 uSize = _uWidth * _uHeight * 4;
	AkU8* pPixels = new AkU8[uSize];
	for (AkU32 i = 0; i < uSize / 4; i++)
	{
		AkF32 fA = _pVertices[i].pAlpha[_iSelectedTexture];
		AkU8 uAlpha = (fA * 255.0f) / MAX_ALPHA;
		pPixels[(4 * i) + 0] = 0;
		pPixels[(4 * i) + 1] = 0;
		pPixels[(4 * i) + 2] = 0;
		pPixels[(4 * i) + 3] = 255;

		pPixels[(4 * i) + _iSelectedTexture] = uAlpha;
	}

	DirectX::Image tImage = {};
	tImage.width = _uWidth;
	tImage.height = _uHeight;
	tImage.pixels = pPixels;
	tImage.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tImage.rowPitch = tImage.width * 4;
	tImage.slicePitch = uSize;

	wchar_t wcTemp[2] = {};
	_itow_s(_iSelectedTexture, wcTemp, 10);
	wchar_t wcPath[_MAX_PATH] = {};
	wcscat_s(wcPath, MAP_FILE_PATH);
	wcscat_s(wcPath, wcAlphaFile);
	wcscat_s(wcPath, L"_");
	wcscat_s(wcPath, wcTemp);
	wcscat_s(wcPath, L".png");

	SaveToWICFile(tImage, WIC_FLAGS_FORCE_RGB, GetWICCodec(WIC_CODEC_PNG), wcPath);

	delete[] pPixels;
	pPixels = nullptr;
}

void TerrainEdit::CleanUp()
{
	DestroyMeshData();
	DestroyRenderObject();
}

void TerrainEdit::CreateMeshData()
{
	DestroyMeshData();

	Vector4* pPixels = nullptr;
	if (_pHeightMapImg)
	{
		AkU32 uSize = _uWidth * _uHeight;
		pPixels = new Vector4[uSize];
		ImageToPixel(_pHeightMapImg, pPixels, uSize * 4);
	}

	AkU32 uMeshDataNum = 0;
	MeshData_t* pGrid = GeometryGenerator::MakeGrid(&uMeshDataNum, (AkF32)_uWidth, (_uWidth - 1), (_uHeight - 1), &_vTexScale); // uMeshDataNum == 1
	for (AkU32 i = 0; i < pGrid->uVerticeNum; i++)
	{
		pGrid->pVertices[i].vPosition = Vector3::Transform(pGrid->pVertices[i].vPosition, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
		pGrid->pVertices[i].vNormalModel = Vector3::Transform(pGrid->pVertices[i].vNormalModel, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
		pGrid->pVertices[i].vTangentModel = Vector3::Transform(pGrid->pVertices[i].vTangentModel, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
	}

	// Change Vertex Type
	_pVertices = new TerrainVertex_t[pGrid->uVerticeNum];
	_pIndices = new AkU32[pGrid->uIndicesNum];
	_uVerticeNum = pGrid->uVerticeNum;
	_uIndiceNum = pGrid->uIndicesNum;

	for (AkU32 i = 0; i < pGrid->uVerticeNum; i++)
	{
		_pVertices[i].vPosition = pGrid->pVertices[i].vPosition;
		if (_pHeightMapImg)
		{
			_pVertices[i].vPosition.y += pPixels[i].x * MAX_HEIGHT;
		}
		_pVertices[i].vNormalModel = pGrid->pVertices[i].vNormalModel;
		_pVertices[i].vTexCoord = pGrid->pVertices[i].vTexCoord;
		_pVertices[i].vTangentModel = pGrid->pVertices[i].vTangentModel;
	}

	memcpy(_pIndices, pGrid->pIndices, sizeof(AkU32) * _uIndiceNum);

	// Delete Origin MeshData.
	if (pGrid)
	{
		GeometryGenerator::DestroyGeometry(pGrid, 1);
		pGrid = nullptr;
	}

	ComputeNormals();
	ComputeTangents();

	if (pPixels)
	{
		delete[] pPixels;
		pPixels = nullptr;
	}
}

void TerrainEdit::CreateRenderObject()
{
	_pTerrain = GRenderer->CreateTerrain();

	_pDVHandle = _pTerrain->CreateDynamicMeshBuffers(_pVertices, _uVerticeNum, _pIndices, _uIndiceNum);
	_pTerrain->SetTextures(L"../../assets/map/grass.dds", L"../../assets/map/stone.dds"); // TODO

	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pTerrain->UpdateMaterialBuffers(&vAlbedo, 0.0f, 1.0f, &vEmissive);
}

void TerrainEdit::DestroyMeshData()
{
	if (_pVertices)
	{
		delete _pVertices;
		_pVertices = nullptr;
	}
	if (_pIndices)
	{
		delete _pIndices;
		_pIndices = nullptr;
	}
}

void TerrainEdit::DestroyRenderObject()
{
	if (_pDVHandle)
	{
		// 삭제 로직 다시 생각해보기
		// terrian 에서 만든 dynamic obj 를 renderer 에서 삭제하는게 맞는것인가??
		GRenderer->DestroyDynamicVertex(_pDVHandle);
		_pDVHandle = nullptr;
	}
	if (_pTerrain)
	{
		_pTerrain->Release();
		_pTerrain = nullptr;
	}
}

void TerrainEdit::ComputeHeight()
{
	if (0 == _tBrush.iType)
	{
		// 구
		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			Vector3 p = Vector3(_pVertices[i].vPosition.x, 0.0f, _pVertices[i].vPosition.z);
			Vector3 c = Vector3(_vPickPos.x, 0, _vPickPos.z);

			AkF32 fDistance = (c - p).Length();
			AkF32 fCosValue = cos(DirectX::XM_PIDIV2 * fDistance / _tBrush.fRange);
			AkF32 fTemp = _fHeightScale * max(0, fCosValue);
			if (fDistance <= _tBrush.fRange)
			{
				if (_bPositive)
				{
					_pVertices[i].vPosition.y += fTemp * DT;
				}
				else
				{
					_pVertices[i].vPosition.y -= fTemp * DT;
					if (MIN_HEIGHT > _pVertices[i].vPosition.y)
					{
						_pVertices[i].vPosition.y = MIN_HEIGHT;
					}
					else if (MAX_HEIGHT > _pVertices[i].vPosition.y)
					{
						_pVertices[i].vPosition.y = MAX_HEIGHT;
					}
				}
			}
		}
	}
	if (1 == _tBrush.iType)
	{
		// 사각형
		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			Vector3 p = Vector3(_pVertices[i].vPosition.x, 0.0f, _pVertices[i].vPosition.z);
			Vector3 c = Vector3(_vPickPos.x, 0, _vPickPos.z);

			float distX = abs(c.x - p.x);
			float distZ = abs(c.z - p.z);

			if (distX <= _tBrush.fRange && distZ <= _tBrush.fRange)
			{
				if (_bPositive)
				{
					_pVertices[i].vPosition.y += _fHeightScale * DT;
				}
				else
				{
					_pVertices[i].vPosition.y -= _fHeightScale * DT;
					if (MIN_HEIGHT > _pVertices[i].vPosition.y)
					{
						_pVertices[i].vPosition.y = MIN_HEIGHT;
					}
				}
			}
		}
	}

	GRenderer->UpdateDynamicVertices(_pDVHandle, _pVertices);
}

void TerrainEdit::PaintBrush()
{
	if (0 == _tBrush.iType)
	{
		// 구
		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			Vector3 p = Vector3(_pVertices[i].vPosition.x, 0.0f, _pVertices[i].vPosition.z);
			Vector3 c = Vector3(_vPickPos.x, 0, _vPickPos.z);

			AkF32 fDistance = (c - p).Length();
			AkF32 fCosValue = cos(DirectX::XM_PIDIV2 * fDistance / _tBrush.fRange);
			AkF32 fTemp = _fPaintScale * max(0, fCosValue);
			if (fDistance <= _tBrush.fRange)
			{
				if (_bPositive)
				{
					_pVertices[i].pAlpha[_iSelectedTexture] += fTemp * DT;
				}
				else
				{
					_pVertices[i].pAlpha[_iSelectedTexture] -= fTemp * DT;
				}

				_pVertices[i].pAlpha[_iSelectedTexture] = Clamp(_pVertices[i].pAlpha[_iSelectedTexture], 0.0f, 1.0f);
			}
		}
	}
	if (1 == _tBrush.iType)
	{
		// 사각형
		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			Vector3 p = Vector3(_pVertices[i].vPosition.x, 0.0f, _pVertices[i].vPosition.z);
			Vector3 c = Vector3(_vPickPos.x, 0, _vPickPos.z);

			float distX = abs(c.x - p.x);
			float distZ = abs(c.z - p.z);

			if (distX <= _tBrush.fRange && distZ <= _tBrush.fRange)
			{
				if (_bPositive)
				{
					_pVertices[i].pAlpha[_iSelectedTexture] += _fPaintScale * DT;
				}
				else
				{
					_pVertices[i].pAlpha[_iSelectedTexture] -= _fPaintScale * DT;
				}

				_pVertices[i].pAlpha[_iSelectedTexture] = Clamp(_pVertices[i].pAlpha[_iSelectedTexture], 0.0f, 1.0f);
			}
		}
	}

	GRenderer->UpdateDynamicVertices(_pDVHandle, _pVertices);
}

void TerrainEdit::ComputeNormals()
{
	DirectX::XMFLOAT3* position = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT3* normal = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT2* texCoord = new DirectX::XMFLOAT2[_uVerticeNum];
	DirectX::XMFLOAT3* tangent = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT3* biTangent = new DirectX::XMFLOAT3[_uVerticeNum];

	for (AkU32 i = 0; i < _uVerticeNum; i++)
	{
		TerrainVertex_t v = {};
		if (_pVertices)
		{
			v = _pVertices[i];
			position[i] = v.vPosition;
		}
	}

	DirectX::ComputeNormals(_pIndices, _uIndiceNum / 3, position, _uVerticeNum, DirectX::CNORM_DEFAULT, normal);

	if (_pVertices)
	{
		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			_pVertices[i].vNormalModel = normal[i];
		}
	}

	delete[] position;
	delete[] normal;
	delete[] texCoord;
	delete[] tangent;
	delete[] biTangent;
}

void TerrainEdit::ComputeTangents()
{
	DirectX::XMFLOAT3* position = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT3* normal = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT2* texCoord = new DirectX::XMFLOAT2[_uVerticeNum];
	DirectX::XMFLOAT3* tangent = new DirectX::XMFLOAT3[_uVerticeNum];
	DirectX::XMFLOAT3* biTangent = new DirectX::XMFLOAT3[_uVerticeNum];

	for (AkU32 i = 0; i < _uVerticeNum; i++)
	{
		TerrainVertex_t v = {};
		if (_pVertices)
		{
			v = _pVertices[i];
			position[i] = v.vPosition;
			normal[i] = v.vNormalModel;
			texCoord[i] = v.vTexCoord;
		}
	}

	DirectX::ComputeTangentFrame(_pIndices, _uIndiceNum / 3, position, normal, texCoord, _uVerticeNum, tangent, biTangent);

	if (_pVertices)
	{
		for (AkU32 i = 0; i < _uVerticeNum; i++)
		{
			_pVertices[i].vTangentModel = tangent[i];
		}
	}

	delete[] position;
	delete[] normal;
	delete[] texCoord;
	delete[] tangent;
	delete[] biTangent;
}

void TerrainEdit::UpdateMousePicking()
{
	if (!_bDrawBrush)
	{
		_tBrush.fRange = 0.0f;
		return;
	}

	Vector3 v0 = Vector3(-(AkF32)_uWidth * 0.5f, 0.0f, -(AkF32)_uHeight * 0.5f);
	Vector3 v1 = Vector3(-(AkF32)_uWidth * 0.5f, 0.0f, _uHeight * 0.5f);
	Vector3 v2 = Vector3((AkF32)_uWidth * 0.5f, 0.0f, _uHeight * 0.5f);
	Vector3 v3 = Vector3((AkF32)_uWidth * 0.5f, 0.0f, -(AkF32)_uHeight * 0.5f);
	AkBool bPicked = GRenderer->MousePickingToSqaure(&v0, &v1, &v2, &v3, NDC_X, NDC_Y, &_vPickPos, &_fPickDist, &_fMoveRatio);

	_tBrush.vPos = _vPickPos;

	if (LBTN_HOLD)
		_bPicked = bPicked;
}
