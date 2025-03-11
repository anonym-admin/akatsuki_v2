#include "pch.h"
#include "Terrain.h"

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
	CreateMeshData();

	// LoadHeightMap(L"Test.map");

	_pMeshObj = GRenderer->CreateBasicMeshObject();
	_pMeshObj->CreateMeshBuffers(_pGrid, 1);

	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissvie = Vector3(0.0f);
	_pMeshObj->UpdateMaterialBuffers(&vAlbedo, 0.0f, 1.0f, &vEmissvie);

	GeometryGenerator::DestroyGeometry(_pGrid, 1);

	// Create transform.
	_pTransform = CreateTransform();

	return AK_TRUE;
}

void Terrain::Update()
{
	DRAW_WIRE ? _pMeshObj->EnableWireFrame() : _pMeshObj->DisableWireFrame();


}

void Terrain::FinalUpdate()
{
	_pTransform->Update();
}

void Terrain::RenderShadow()
{
	GRenderer->RenderShadowOfBasicMeshObject(_pMeshObj, &_pTransform->GetWorldTransform());
}

void Terrain::Render()
{
	GRenderer->RenderBasicMeshObject(_pMeshObj, &_pTransform->GetWorldTransform());
}

void Terrain::OnCollision(Collider* pOther)
{
}

void Terrain::OnCollisionEnter(Collider* pOther)
{
}

void Terrain::OnCollisionExit(Collider* pOther)
{
}

void Terrain::CleanUp()
{
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}
}

void Terrain::CreateMeshData()
{
	AkU32 uMeshDataNum = 0;
	_pGrid = GeometryGenerator::MakeGrid(&uMeshDataNum, (AkF32)_uWidth, _uWidth, _uHeight); // uMeshDataNum == 1
	for (AkU32 i = 0; i < _pGrid->uVerticeNum; i++)
	{
		_pGrid->pVertices[i].vPosition = Vector3::Transform(_pGrid->pVertices[i].vPosition, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
		_pGrid->pVertices[i].vNormalModel = Vector3::Transform(_pGrid->pVertices[i].vNormalModel, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
		_pGrid->pVertices[i].vTangentModel = Vector3::Transform(_pGrid->pVertices[i].vTangentModel, Matrix::CreateRotationX(DirectX::XM_PIDIV2));
	}
}

void Terrain::LoadHeightMap(const wchar_t* wcHeightFile)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcHeightFile, L"rb");
	if (!fp)
	{
		__debugbreak();
	}

	fwscanf_s(fp, L"%u", &_pGrid->uVerticeNum);
	fwscanf_s(fp, L"%u", &_pGrid->uIndicesNum);
	for (AkU32 i = 0; i < _pGrid->uVerticeNum; i++)
	{
		fwscanf_s(fp, L"%f %f %f", &_pGrid->pVertices[i].vPosition.x, &_pGrid->pVertices[i].vPosition.y, &_pGrid->pVertices[i].vPosition.z);
		fwscanf_s(fp, L"%f %f %f", &_pGrid->pVertices[i].vNormalModel.x, &_pGrid->pVertices[i].vNormalModel.y, &_pGrid->pVertices[i].vNormalModel.z);
		fwscanf_s(fp, L"%f %f %f", &_pGrid->pVertices[i].vTangentModel.x, &_pGrid->pVertices[i].vTangentModel.y, &_pGrid->pVertices[i].vTangentModel.z);
		fwscanf_s(fp, L"%f %f", &_pGrid->pVertices[i].vTexCoord.x, &_pGrid->pVertices[i].vTexCoord.y);
	}

	if (fp)
	{
		fclose(fp);
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

	_pTerrain = GRenderer->CreateTerrain();

	_pDVHandle = _pTerrain->CreateDynamicMeshBuffers(_pVertices, _uVerticeNum, _pIndices, _uIndiceNum);
	_pTerrain->SetTextures(L"../../assets/grass.dds", L"../../assets/stone.dds");

	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pTerrain->UpdateMaterialBuffers(&vAlbedo, 0.0f, 1.0f, &vEmissive);

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
			else if(_iEditType == 1)
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
	ImGui::SliderFloat("Set Minimize Hegiht", &_fHeightMin, -100.0f, 0.0f);

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

void TerrainEdit::Load(const wchar_t* wcHeightFile)
{
	//FILE* fp = nullptr;
	//_wfopen_s(&fp, wcHeightFile, L"rb");
	//if (!fp)
	//{
	//	__debugbreak();
	//}

	//fwscanf_s(fp, L"%u", &_pGrid->uVerticeNum);
	//fwscanf_s(fp, L"%u", &_pGrid->uIndicesNum);
	//for (AkU32 i = 0; i < _pGrid->uVerticeNum; i++)
	//{
	//	fwscanf_s(fp, L"%f %f %f",& _pGrid->pVertices[i].vPosition.x, &_pGrid->pVertices[i].vPosition.y, &_pGrid->pVertices[i].vPosition.z);
	//	fwscanf_s(fp, L"%f %f %f", &_pGrid->pVertices[i].vNormalModel.x, &_pGrid->pVertices[i].vNormalModel.y, &_pGrid->pVertices[i].vNormalModel.z);
	//	fwscanf_s(fp, L"%f %f %f", &_pGrid->pVertices[i].vTangentModel.x, &_pGrid->pVertices[i].vTangentModel.y, &_pGrid->pVertices[i].vTangentModel.z);
	//	fwscanf_s(fp, L"%f %f", &_pGrid->pVertices[i].vTexCoord.x, &_pGrid->pVertices[i].vTexCoord.y);
	//}

	//if (fp)
	//{
	//	fclose(fp);
	//}
}

void TerrainEdit::Save(const wchar_t* wcHeightFile)
{
	//FILE* fp = nullptr;
	//_wfopen_s(&fp, wcHeightFile, L"wb");
	//if (!fp)
	//{
	//	__debugbreak();
	//}

	//fwprintf_s(fp, L"%u\n", _pGrid->uVerticeNum);
	//fwprintf_s(fp, L"%u\n", _pGrid->uIndicesNum);
	//for (AkU32 i = 0; i < _pGrid->uVerticeNum; i++)
	//{
	//	fwprintf_s(fp, L"%lf %lf %lf\n", _pGrid->pVertices[i].vPosition.x, _pGrid->pVertices[i].vPosition.y, _pGrid->pVertices[i].vPosition.z);
	//	fwprintf_s(fp, L"%lf %lf %lf\n", _pGrid->pVertices[i].vNormalModel.x, _pGrid->pVertices[i].vNormalModel.y, _pGrid->pVertices[i].vNormalModel.z);
	//	fwprintf_s(fp, L"%lf %lf %lf\n", _pGrid->pVertices[i].vTangentModel.x, _pGrid->pVertices[i].vTangentModel.y, _pGrid->pVertices[i].vTangentModel.z);
	//	fwprintf_s(fp, L"%lf %lf\n", _pGrid->pVertices[i].vTexCoord.x, _pGrid->pVertices[i].vTexCoord.y);
	//}

	//if (fp)
	//{
	//	fclose(fp);
	//}
}

void TerrainEdit::CleanUp()
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

void TerrainEdit::CreateMeshData()
{
	AkU32 uMeshDataNum = 0;
	MeshData_t* pGrid = GeometryGenerator::MakeGrid(&uMeshDataNum, (AkF32)_uWidth, _uWidth, _uHeight); // uMeshDataNum == 1
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
					if (_fHeightMin > _pVertices[i].vPosition.y)
					{
						_pVertices[i].vPosition.y = _fHeightMin;
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
					if (_fHeightMin > _pVertices[i].vPosition.y)
					{
						_pVertices[i].vPosition.y = _fHeightMin;
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

	if(LBTN_HOLD)
		_bPicked = bPicked;
}
