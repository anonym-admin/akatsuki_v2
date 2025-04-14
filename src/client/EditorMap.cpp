#include "pch.h"
#include "EditorMap.h"
#include "Camera.h"
#include "Terrain.h"
#include "ModelObject.h"
#include "Scene.h"
#include "TreeBillboards.h"
#include "Light.h"

/*
=============
Editor Map
=============
*/

EditorMap::EditorMap()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

EditorMap::~EditorMap()
{
	CleanUp();
}

AkBool EditorMap::Initialize()
{
	Vector3 vCamPos = Vector3(100.0f, 100.0f, -150.0f);
	Vector3 vCamYawPitchRoll = Vector3(-0.6f, 0.5f, 0.0f);
	_pCamera = new Camera(&vCamPos, &vCamYawPitchRoll);
	_pCamera->Mode = CAMERA_MODE::EDITOR;

	// Editor 에서는 Scene에 Obj 를 등록하지 않는다.
	_pTerrainEdit = new TerrainEdit;

	// Create ocean.
	_pOcean = GRenderer->CreateOceanObject();

	// Create cloud.
	_pCloud = GRenderer->CreateCloudObject();

	// Clipper.
	_pCSGCube = GRenderer->CreateBasicMeshObject();

	Vector3 vMin = Vector3(-0.5f);
	Vector3 vMax = Vector3(0.5f);
	_pCube = GeometryGenerator::MakeCube(&vMin, &vMax);
	_pCSGDBHandle = _pCSGCube->CreateDynamicMeshBuffers(_pCube->pVertices, _pCube->uVerticeNum, _pCube->pIndices, _pCube->uIndicesNum);

	return AK_TRUE;
}

AkBool EditorMap::BeginEditor()
{
	// Scene 또는 다른 Editor 에서 전환된 카메라 위치 방향 조정.
	// TODO!!
	_pCamera->GetTransform()->SetPosition(100.0f, 100.0f, -150.0f);
	_pCamera->GetTransform()->SetRotation(-0.6f, 0.5f, 0.0f);
	_bFPV = AK_FALSE;

	Collider::DRAW_COLLIDER = AK_TRUE;

	return AK_TRUE;
}

AkBool EditorMap::EndEditor()
{
	Collider::DRAW_COLLIDER = AK_FALSE;

	return AK_TRUE;
}

void EditorMap::Update()
{
	UpdateControl();

	if (_bFPV)
	{
		_pCamera->Update();
	}

	_pTerrainEdit->Update();

	// Update Lights
	for (auto& e : _vecLights)
	{
		e->Update();
	}

	// Update
	for (auto& e : _vecGameObj)
	{
		e->Update();
	}

	// Update Tree Billboard
	for (auto& e : _mapBillboard)
	{
		if (e.second)
		{
			e.second->Update();
		}
	}

	// Final Update
	for (auto& e : _vecGameObj)
	{
		e->FinalUpdate();
	}

	// Final Update Tree Billboard
	for (auto& e : _mapBillboard)
	{
		if (e.second)
		{
			e.second->FinalUpdate();
		}
	}

	GRenderer->UpdateDynamicVertexBuffer(_pCSGDBHandle, _pCube->pVertices);
}

void EditorMap::Render()
{
	UpdateGizmo();

	for (auto& e : _vecLights)
	{
		e->Render();
	}

	_pTerrainEdit->Render();

	for (auto& e : _vecGameObj)
	{
		e->Render();
	}

	for(auto& e : _mapBillboard)
	{
		if(e.second)
		{
			e.second->Render();
		}
	}

	Matrix world = Matrix::CreateRotationX(DirectX::XM_PIDIV2) * Matrix::CreateTranslation(Vector3(0.0f, 0.5f, 0.0f));

	// Render Ocean.
	GRenderer->RenderOcean(_pOcean, GTimer->GetTotalTime(), &world);

	// Render Cloud.
	GRenderer->RenderCloud(_pCloud);

	// CSG
	_mCSGWorldRow = Matrix::CreateTranslation(Vector3(0.0f, 1.0f, 0.0f));

	GRenderer->RenderBasicMeshObject(_pCSGCube, &_mCSGWorldRow);
}

void EditorMap::RenderShadowMaps()
{
	for (auto& e : _vecGameObj)
	{
		e->RenderShadowMaps();
	}

	for (auto& e : _mapBillboard)
	{
		if (e.second)
		{
			e.second->RenderShadowMaps();
		}
	}
}

void EditorMap::RenderGUI()
{
	_pCamera->RenderGUI();
	_pTerrainEdit->UpdateEditor();

	IGFD::FileDialogConfig tConfig = {};
	tConfig.filePathName = "../../assets/";

	ImGui::Begin("[Map Editor]");
	ImGui::Checkbox("FPV", &_bFPV);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", DT, FPS);

	const char* pTex[] = { "Albedo", "Normal", "Emissive", "Metallic", "Roughness", "AO", "Second", "Third" };
	if (ImGui::Combo("Texture Type", &_iTextureType, pTex, IM_ARRAYSIZE(pTex)))
	{
		_bLoad = AK_TRUE;
		_iSelectMode = 2;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg,.dds", tConfig);
	}
	if (ImGui::Button("Load Height"))
	{
		_bLoad = AK_TRUE;
		_iSelectMode = 0;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg,.dds", tConfig);
	}
	if (ImGui::Button("Save Height"))
	{
		_bSave = AK_TRUE;
		_iSelectMode = 0;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg,.dds", tConfig);
	}
	if (ImGui::Button("Load Splatting"))
	{
		_bLoad = AK_TRUE;
		_iSelectMode = 1;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg,.dds", tConfig);
	}
	if (ImGui::Button("Save Splatting"))
	{
		_bSave = AK_TRUE;
		_iSelectMode = 1;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".png,.jpg,.dds", tConfig);
	}
	if (ImGui::Button("Load Map"))
	{
		_bLoad = AK_TRUE;
		_iSelectMode = 3;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".map", tConfig);
	}
	if (ImGui::Button("Save Map"))
	{
		_bSave = AK_TRUE;
		_iSelectMode = 3;
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".map", tConfig);
	}

	ImGui::End();

	ImGui::Begin("Actor Info");
	if (ImGui::Button("Import Actor"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ImportActorData", "Choose File", ".act", tConfig);
	}
	ImGui::End();

	ImGui::Begin("Scene Info");
	if (ImGui::Button("Load Scene"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("SceneInfoLoad", "Choose File", ".scene", tConfig);
	}
	if (ImGui::Button("Save Scene"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("SceneInfoSave", "Choose File", ".scene", tConfig);
	}
	ImGui::End();

	ImGui::Begin("Billboard");
	const char* pBillboardItems[] = { "Tree", "Grass" };
	if (ImGui::Combo("Billboard Type", &_iBillboardType, pBillboardItems, IM_ARRAYSIZE(pBillboardItems)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("LoadBillboard", "Choose File", ".dds", tConfig);
	}
	ImGui::End();

	ImGui::Begin("Light");
	if (ImGui::Button("Add Light"))
	{
		CreateLight();
	}
	ImGui::End();

	UpdateFileDialog();
}

void EditorMap::Load(const std::wstring& wcFilePath)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFilePath.c_str(), L"rt");
	if (!fp)
	{
		__debugbreak();
	}

	if (_pTerrainEdit)
	{
		delete _pTerrainEdit;
		_pTerrainEdit = nullptr;
	}

	for (auto& e : _vecGameObj)
	{
		delete e;
		e = nullptr;
	}

	// 01. map files.
	wchar_t wcName[_MAX_PATH] = {};
	fwscanf_s(fp, L"%s", wcName, (unsigned)_MAX_PATH);
	_pTerrainEdit = new TerrainEdit(wcName);
	_wcMapFileName = GetFileName(wcName);

	// 02. act files.
	AkI32 iNumGameObj = 0;
	fwscanf_s(fp, L"%d", &iNumGameObj);
	for(AkI32 i = 0; i < iNumGameObj; i++)
	{
		fwscanf_s(fp, L"%s", wcName, (unsigned)_MAX_PATH);

		ModelObject* pObj = nullptr;
		AkU32 uIndex = 0;
		AkBool bIsInstance = AK_FALSE;
		for (auto& v : _vecActFileNameList)
		{
			if (v == wcName)
			{
				pObj = ((ModelObject*)_vecGameObj[uIndex])->Clone();
				bIsInstance = AK_TRUE;

				_vecGameObj.push_back(pObj);
			}

			uIndex++;
		}

		if(!bIsInstance)
		{
			pObj = new ModelObject(wcName);

			_vecGameObj.push_back(pObj);
		}

		pObj->SetEditMode(AK_TRUE);
		Vector3 vScale = Vector3(1.0f);
		Vector3 vYawPitchRoll = Vector3(0.0f);
		Vector3 vPos = Vector3(0.0f);

		fwscanf_s(fp, L"%f %f %f", &vScale.x, &vScale.y, &vScale.z);	
		fwscanf_s(fp, L"%f %f %f", &vYawPitchRoll.x, &vYawPitchRoll.y, &vYawPitchRoll.z);
		fwscanf_s(fp, L"%f %f %f", &vPos.x, &vPos.y, &vPos.z);
	
		pObj->GetTransform()->SetScale(&vScale);
		pObj->GetTransform()->SetRotation(&vYawPitchRoll);
		pObj->GetTransform()->SetPosition(&vPos);

		pObj->GetTransform()->Update();
	
		_vecActFileNameList.push_back(wcName);
	}

	// 03. billboard.
	AkI32 iNumBillboard = 0;
	fwscanf_s(fp, L"%d", &iNumBillboard);
	for (AkI32 i = 0; i < iNumBillboard; i++)
	{
		fwscanf_s(fp, L"%s", wcName, (unsigned)_MAX_PATH);

		AkI32 iVertexNum = 0;
		fwscanf_s(fp, L"%d", &iVertexNum);

		VertexSize_t* pVertices = new VertexSize_t[iVertexNum];
		for (AkI32 i = 0; i < iVertexNum; i++)
		{
			fwscanf_s(fp, L"%f %f %f", &pVertices[i].vPosition.x, &pVertices[i].vPosition.y, &pVertices[i].vPosition.z);
			fwscanf_s(fp, L"%f %f", &pVertices[i].vSize.x, &pVertices[i].vSize.y);
		}

		Billboard* pBillboard = new Billboard(wcName, pVertices, iVertexNum);
		_mapBillboard[wcName] = pBillboard;
		memcpy(_mapVertices[wcName].data(), pVertices, sizeof(VertexSize_t) * iVertexNum);

		if (pVertices)
		{
			delete[] pVertices;
			pVertices = nullptr;
		}
	}

	if (fp)
	{
		fclose(fp);
		fp = nullptr;
	}
}

void EditorMap::Save(const std::wstring& wcFilePath)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFilePath.c_str(), L"wt");
	if (!fp)
	{
		__debugbreak();
	}

	// 01. map files.
	std::wstring wcFullPath = MAP_FILE_PATH + _wcMapFileName;
	fwprintf_s(fp, L"%s\n", wcFullPath.c_str());

	// 02. act files.
	fwprintf_s(fp, L"%d\n", (AkI32)_vecActFileNameList.size());
	AkU32 uIndex = 0;
	for (auto& e : _vecActFileNameList)
	{
		Vector3 vScale = _vecGameObj[uIndex]->GetTransform()->GetScale();
		Vector3 vYawPitchRoll = _vecGameObj[uIndex]->GetTransform()->GetRotation();
		Vector3 vPos = _vecGameObj[uIndex]->GetTransform()->GetPosition();

		fwprintf_s(fp, L"%s\n", e.c_str());
		fwprintf_s(fp, L"%lf %lf %lf\n", vScale.x, vScale.y, vScale.z);
		fwprintf_s(fp, L"%lf %lf %lf\n", vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z);
		fwprintf_s(fp, L"%lf %lf %lf\n", vPos.x, vPos.y, vPos.z);

		uIndex++;
	}

	// 03. billboard.
	fwprintf_s(fp, L"%d\n", (AkI32)_mapBillboard.size());
	for (auto& e : _mapBillboard)
	{
		fwprintf_s(fp, L"%s\n", e.first.c_str());
		if (_mapVertices.count(e.first))
		{
			fwprintf_s(fp, L"%d\n", (AkI32)_mapVertices[e.first].size());
			for (auto& v : _mapVertices[e.first])
			{
				fwprintf_s(fp, L"%lf %lf %lf\n", v.vPosition.x, v.vPosition.y, v.vPosition.z);
				fwprintf_s(fp, L"%lf %lf\n", v.vSize.x, v.vSize.y);
			}
		}
	}

	if (fp)
	{
		fclose(fp);
		fp = nullptr;
	}
}

void EditorMap::CleanUp()
{
	if (_pCSGDBHandle)
	{
		_pCSGCube->DestoryDynamicVertexBuferHandle(_pCSGDBHandle);
		_pCSGDBHandle = nullptr;
	}
	if (_pCube)
	{
		GeometryGenerator::DestroyGeometry(_pCube, 1);
		_pCube = nullptr;
	}
	if (_pCSGCube)
	{
		_pCSGCube->Release();
		_pCSGCube = nullptr;
	}
	if (_pCloud)
	{
		_pCloud->Release();
		_pCloud = nullptr;
	}
	if (_pOcean)
	{
		_pOcean->Release();
		_pOcean = nullptr;
	}

	for (auto& e : _vecLights)
	{
		delete e;
		e = nullptr;
	}
	_vecLights.clear();

	for (auto& e : _mapBillboard)
	{
		if (e.second)
		{
			delete e.second;
			e.second = nullptr;
		}
	}

	for (auto& e : _vecGameObj)
	{
		delete e;
		e = nullptr;
	}
	_vecGameObj.clear();

	if (_pTerrainEdit)
	{
		delete _pTerrainEdit;
		_pTerrainEdit = nullptr;
	}
	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}
}

Billboard* EditorMap::CreateBillboards(const std::wstring& wcFilePath, VertexSize_t* pVertices, AkU32 uNum)
{
	Billboard* pTreeBilloards = new Billboard(wcFilePath.c_str(), pVertices, uNum);
	return pTreeBilloards;
}

Light* EditorMap::CreateLight()
{
	Vector3 vRadiance = Vector3(1.0f);
	Vector3 vPosition = Vector3(0.0f, 2.5f, -25.0f);
	Light* pPointLight = new Light(LIGHT_TYPE::POINT, &vRadiance, &vPosition, 0.0f, AK_FALSE);
	_vecLights.push_back(pPointLight);

	return nullptr;
}

void EditorMap::ExportMap(const std::wstring& wcFilePath)
{
	FILE* fp = nullptr;

	_wcMapFileName = _wcFileName;
	std::wstring wcFullPath = MAP_FILE_PATH + wcFilePath;

	_wfopen_s(&fp, wcFullPath.c_str(), L"wt");
	if (!fp)
	{
		__debugbreak();
	}

	// 01. Texture name
	fwprintf_s(fp, L"%s\n", _wcAlbedoFilename.empty() ? L"None" : _wcAlbedoFilename.c_str());
	fwprintf_s(fp, L"%s\n", _wcNormalFilename.empty() ? L"None" : _wcNormalFilename.c_str());
	fwprintf_s(fp, L"%s\n", _wcEmissiveFilename.empty() ? L"None" : _wcEmissiveFilename.c_str());
	fwprintf_s(fp, L"%s\n", _wcMetallicFilename.empty() ? L"None" : _wcMetallicFilename.c_str());
	fwprintf_s(fp, L"%s\n", _wcRoughnessFilename.empty() ? L"None" : _wcRoughnessFilename.c_str());
	fwprintf_s(fp, L"%s\n", _wcAOFilename.empty() ? L"None" : _wcAOFilename.c_str());
	fwprintf_s(fp, L"%s\n", _wcSecondFilename.empty() ? L"None" : _wcSecondFilename.c_str());
	fwprintf_s(fp, L"%s\n", _wcThirdFilename.empty() ? L"None" : _wcThirdFilename.c_str());
	// 02. Height Map
	fwprintf_s(fp, L"%s\n", _wcHeightFilename.empty() ? L"None" : _wcHeightFilename.c_str());
	// 03. Splatting Alpha Map
	fwprintf_s(fp, L"%s\n", _wcAlphaFilenames[0].empty() ? L"None" : _wcAlphaFilenames[0].c_str());
	fwprintf_s(fp, L"%s\n", _wcAlphaFilenames[1].empty() ? L"None" : _wcAlphaFilenames[1].c_str());

	if (fp)
	{
		fclose(fp);
		fp = nullptr;
	}
}

void EditorMap::ImportMap(const std::wstring& wcFilePath)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFilePath.c_str(), L"wt");
	if (!fp)
	{
		__debugbreak();
	}

	if (_pTerrainEdit)
	{
		delete _pTerrainEdit;
		_pTerrainEdit = nullptr;
	}

	_pTerrainEdit = new TerrainEdit(wcFilePath.c_str());
	_wcMapFileName = wcFilePath;

	if (fp)
	{
		fclose(fp);
		fp = nullptr;
	}
}

void EditorMap::ImportActor(const std::wstring& wcFilePath)
{
	ModelObject* pObj = new ModelObject(wcFilePath.c_str());
	pObj->SetEditMode(AK_TRUE);
	_vecGameObj.push_back(pObj);
	_vecActFileNameList.push_back(wcFilePath);
}

void EditorMap::UpdateControl()
{
	if (KEY_DOWN(KEY_INPUT_F))
	{
		_bFPV = !_bFPV;
	}
}

void EditorMap::UpdateFileDialog()
{
	// display
	if (_bLoad)
	{
		if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
		{
			if (ImGuiFileDialog::Instance()->IsOk() == true)
			{
				std::string cFilename = GetFileName(ImGuiFileDialog::Instance()->GetFilePathName());
				std::string cFilenameExcludeExt = GetFileNmaeExcludeExt(cFilename);
				std::string cFilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

				_wcFileName.assign(cFilename.begin(), cFilename.end());
				_wcFileNameExcExt.assign(cFilenameExcludeExt.begin(), cFilenameExcludeExt.end());
				_wcFilePath.assign(cFilePath.begin(), cFilePath.end());

				if (!_wcFileName.length())
				{
					ImGuiFileDialog::Instance()->Close();
					_bLoad = AK_FALSE;
					return;
				}

				switch (_iSelectMode)
				{
				case 0:
				{
					_pTerrainEdit->LoadHeightMap(_wcFileNameExcExt.c_str());
					_wcHeightFilename = _wcFileNameExcExt;
					break;
				}
				case 1:
				{
					AkI32 iSelectedID = 0;
					_pTerrainEdit->LoadSplatingTexture(_wcFileNameExcExt.c_str(), &iSelectedID);
					_wcAlphaFilenames[iSelectedID] = _wcFileNameExcExt;
					break;
				}
				case 2:
				{
					if (0 == _iTextureType) // Albedo
					{
						_wcAlbedoFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures((_wcFilePath + _wcFileName).c_str(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
					}
					else if (1 == _iTextureType) // Normal
					{
						_wcNormalFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, (_wcFilePath + _wcFileName).c_str(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
					}
					else if (2 == _iTextureType) // Emissive
					{
						_wcEmissiveFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, nullptr, (_wcFilePath + _wcFileName).c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
					}
					else if (3 == _iTextureType) // Metallic
					{
						_wcMetallicFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, nullptr, nullptr, (_wcFilePath + _wcFileName).c_str(), nullptr, nullptr, nullptr, nullptr);
					}
					else if (4 == _iTextureType) // Roughness
					{
						_wcRoughnessFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, nullptr, nullptr, nullptr, (_wcFilePath + _wcFileName).c_str(), nullptr, nullptr, nullptr);
					}
					else if (5 == _iTextureType) // AO
					{
						_wcAOFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, nullptr, nullptr, nullptr, nullptr, (_wcFilePath + _wcFileName).c_str(), nullptr, nullptr);
					}
					else if (6 == _iTextureType) // Second
					{
						_wcSecondFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, (_wcFilePath + _wcFileName).c_str(), nullptr);
					}
					else if (7 == _iTextureType) // Third
					{
						_wcThirdFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, (_wcFilePath + _wcFileName).c_str());
					}
					break;
				}
				case 3:
				{
					ImportMap((_wcFileName).c_str());
					break;
				}
				default:
				{
					__debugbreak();
					break;
				}
				}
			}

			// close
			ImGuiFileDialog::Instance()->Close();
			_bLoad = AK_FALSE;
		}
	}
	else if (_bSave)
	{
		if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
		{
			if (ImGuiFileDialog::Instance()->IsOk() == true)
			{
				std::string cFilename = GetFileName(ImGuiFileDialog::Instance()->GetFilePathName());
				std::string cFilenameExcludeExt = GetFileNmaeExcludeExt(cFilename);
				std::string cFilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

				_wcFileName.assign(cFilename.begin(), cFilename.end());
				_wcFileNameExcExt.assign(cFilenameExcludeExt.begin(), cFilenameExcludeExt.end());
				_wcFilePath.assign(cFilePath.begin(), cFilePath.end());

				if (!_wcFileName.length())
				{
					ImGuiFileDialog::Instance()->Close();
					_bLoad = AK_FALSE;
					return;
				}

				switch (_iSelectMode)
				{
				case 0:
				{
					_pTerrainEdit->SaveHeightMap(_wcFileNameExcExt.c_str());
					_wcHeightFilename = _wcFileNameExcExt;
					break;
				}
				case 1:
				{
					AkI32 iSelectedID = 0;
					_pTerrainEdit->SaveSplatingTexture(_wcFileNameExcExt.c_str(), &iSelectedID);
					_wcAlphaFilenames[iSelectedID] = _wcFileNameExcExt + L"_" + std::to_wstring(iSelectedID);
					break;
				}
				case 2:
					// Don`t save texture.
					break;
				case 3:
				{
					ExportMap((_wcFileName).c_str());
					break;
				}
				default:
				{
					__debugbreak();
					break;
				}
				}
			}

			// close
			ImGuiFileDialog::Instance()->Close();
			_bSave = AK_FALSE;
		}
	}
	// Import Actor File Dialog.
	if (ImGuiFileDialog::Instance()->Display("ImportActorData"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

			ImportActor(ToWString(FileName));
		}

		ImGuiFileDialog::Instance()->Close();
	}
	// Load Scene File.
	if (ImGuiFileDialog::Instance()->Display("SceneInfoLoad"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

			Load(ToWString(FileName));
		}

		ImGuiFileDialog::Instance()->Close();
	}
	// Save Scene File.
	if (ImGuiFileDialog::Instance()->Display("SceneInfoSave"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

			CreateFolders(FilePath);

			Save(ToWString(FileName));
		}

		ImGuiFileDialog::Instance()->Close();
	}
	// Save Scene File.
	if (ImGuiFileDialog::Instance()->Display("LoadBillboard"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

			std::array<VertexSize_t, 20> _arrVertices = {};
			for (AkU32 i = 0; i < _arrVertices.size(); i++)
			{
				AkF32 fX = Random(-50.0f, 50.0f);
				AkF32 fY = 0.0f;
				AkF32 fZ = Random(-50.0f, 50.0f);

				_arrVertices[i].vPosition = Vector4(fX, fY, fZ, 1.0f);
				_arrVertices[i].vSize = Vector2(1.0f);
			}

			Billboard* pBillboard = CreateBillboards(ToWString(FileName), _arrVertices.data(), (AkU32)_arrVertices.size());

			_mapVertices[ToWString(FileName)] = _arrVertices;
			_mapBillboard[ToWString(FileName)] = pBillboard;
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void EditorMap::UpdateGizmo()
{
	for (auto& e : _vecGameObj)
	{
		((ModelObject*)e)->RenderGUI();
	}

	for (auto& e : _vecLights)
	{
		e->RenderGUI();
	}
}

