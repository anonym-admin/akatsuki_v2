#include "pch.h"
#include "EditorMap.h"
#include "Camera.h"
#include "Terrain.h"
#include "ModelObject.h"
#include "Scene.h"
#include "TreeBillboards.h"
#include "Light.h"
#include "Ocean.h"
#include "Cloud.h"
#include "TreeModel.h"
#include "Tree.h"

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

	//// CSG Clipper.
	//_pCSGCube = GRenderer->CreateBasicMeshObject();
	//_pCube = GeometryGenerator::MakeCube(&vMin, &vMax);
	//_pCSGDBHandle = _pCSGCube->CreateDynamicMeshBuffers(_pCube->pVertices, _pCube->uVerticeNum, _pCube->pIndices, _pCube->uIndicesNum);

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

	// Update game object.
	UpdateObject();

	// Update Tree Billboard
	for (auto& e : _mapBillboard)
	{
		if (e.second)
		{
			e.second->Update();
		}
	}
	
	// Final update game object.
	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			v.first->FinalUpdate();
		}
	}

	// Final Update Tree Billboard
	for (auto& e : _mapBillboard)
	{
		if (e.second)
		{
			e.second->FinalUpdate();
		}
	}

	// Delete Process.
	UpdateObjectDeleteState();

	//// Update CSG
	// GRenderer->UpdateDynamicVertexBuffer(_pCSGDBHandle, _pCube->pVertices);
}

void EditorMap::Render()
{
	UpdateGizmo();

	for (auto& e : _vecLights)
	{
		e->Render();
	}

	_pTerrainEdit->Render();

	// Render game obj.
	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			v.first->Render();
		}
	}

	// Render billboard obj.
	for (auto& e : _mapBillboard)
	{
		if (e.second)
		{
			e.second->Render();
		}
	}

	//// CSG
	//Matrix world = Matrix::CreateRotationX(DirectX::XM_PIDIV2) * Matrix::CreateTranslation(Vector3(0.0f, 0.5f, 0.0f));
	// _mCSGWorldRow = Matrix::CreateTranslation(Vector3(0.0f, 1.0f, 0.0f));
	// GRenderer->RenderBasicMeshObject(_pCSGCube, &_mCSGWorldRow);
}

void EditorMap::RenderShadowMaps()
{
	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			v.first->RenderShadowMaps();
		}
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

	// Create Map Object.
	ImGui::Begin("Map Object");
	{
		const char* pBillboardItems[] = { "Tree", "Grass" };
		if (ImGui::Combo("Billboard Type", &_iBillboardType, pBillboardItems, IM_ARRAYSIZE(pBillboardItems)))
		{
			ImGuiFileDialog::Instance()->OpenDialog("LoadBillboard", "Choose File", ".dds", tConfig);
		}
		// Create Ocean.
		if (ImGui::Button("Create Ocean"))
		{
			Ocean* pOcean = CreateOcean();
			// _vecGameObj.push_back(pOcean);

			_mapGameObj[pOcean->Name].push_back(std::make_pair(pOcean, false));
		}
		// Create Cloud.
		if (ImGui::Button("Create Cloud"))
		{
			Cloud* pCloud = CreateCloud();
			// _vecGameObj.push_back(pCloud);

			_mapGameObj[pCloud->Name].push_back(std::make_pair(pCloud, false));
		}
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

	//// TODO!!
	//for (auto& e : _vecGameObj)
	//{
	//	delete e;
	//	e = nullptr;
	//}

	// 01. map files.
	wchar_t wcName[_MAX_PATH] = {};
	fwscanf_s(fp, L"%s", wcName, (unsigned)_MAX_PATH);
	_pTerrainEdit = new TerrainEdit(wcName);
	_wcMapFileName = GetFileName(wcName);

	// 02. act files.
	AkI32 iNumGameObj = 0;
	fwscanf_s(fp, L"%d", &iNumGameObj);
	for (AkI32 i = 0; i < iNumGameObj; i++)
	{
		fwscanf_s(fp, L"%s", wcName, (unsigned)_MAX_PATH);

		ModelObject* pObj = nullptr;
		AkBool bIsTree = AK_FALSE;

		// 이미 같은 이름으로 생성된 오브젝트가 있다면 인스턴스화를 시킨다.
		std::wstring wcTempName = wcName;
		if (_mapGameObj.count(wcTempName))
		{
			if (_mapGameObj[wcTempName].empty())
				__debugbreak();

			pObj = ((ModelObject*)_mapGameObj[wcTempName][0].first)->Clone();

			_mapGameObj[wcTempName].push_back(std::make_pair(pObj, false));
		}
		else
		{
			// Ocean 생성
			if (wcTempName.find(L"Ocean") != std::wstring::npos)
			{
				pObj = CreateOcean();
			}
			// Cloud 생성
			else if (wcTempName.find(L"Cloud") != std::wstring::npos)
			{
				pObj = CreateCloud();
			}
			// Tree 생성
			else if (wcTempName.find(L"tree") != std::wstring::npos)
			{
				pObj = new Tree(wcTempName.c_str());
				bIsTree = AK_TRUE;
			}
			// 그 외에 모델 오브젝트 생성 
			else
			{
				// 모델 파일 이름을 전달해서 생성시도.
				pObj = new ModelObject(wcTempName.c_str());
			}

			_mapGameObj[wcTempName].push_back(std::make_pair(pObj, false));
		}

		// 에디트 모드 플래그를 통해 ImGui 와 ImGizmo 컨트롤 가능.
		pObj->SetEditMode(AK_TRUE);

		Vector3 vScale = Vector3(1.0f);
		Vector3 vYawPitchRoll = Vector3(0.0f);
		Vector3 vPos = Vector3(0.0f);
		AkF32 fWindTrunk = 0.0f;
		AkF32 fWindLeaves = 0.0f;

		fwscanf_s(fp, L"%f %f %f", &vScale.x, &vScale.y, &vScale.z);
		fwscanf_s(fp, L"%f %f %f", &vYawPitchRoll.x, &vYawPitchRoll.y, &vYawPitchRoll.z);
		fwscanf_s(fp, L"%f %f %f", &vPos.x, &vPos.y, &vPos.z);
		if (bIsTree)
		{
			fwscanf_s(fp, L"%f", &fWindTrunk);
			fwscanf_s(fp, L"%f", &fWindLeaves);

			TreeModel* pTemp = (TreeModel*)pObj->GetModel();
			pTemp->SetWindTrunk(fWindTrunk);
			pTemp->SetWindLeaves(fWindLeaves);
		}

		pObj->GetTransform()->SetScale(&vScale);
		pObj->GetTransform()->SetRotation(&vYawPitchRoll);
		pObj->GetTransform()->SetPosition(&vPos);

		pObj->GetTransform()->Update();
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
	AkU32 uGameObjSize = 0;
	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			uGameObjSize++;
		}
	}
	
	fwprintf_s(fp, L"%d\n", (AkI32)uGameObjSize);
	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			Vector3 vScale = v.first->GetTransform()->GetScale();
			Vector3 vYawPitchRoll = v.first->GetTransform()->GetRotation();
			Vector3 vPos = v.first->GetTransform()->GetPosition();

			fwprintf_s(fp, L"%s \n", e.first.c_str());
			fwprintf_s(fp, L"%lf %lf %lf\n", vScale.x, vScale.y, vScale.z);
			fwprintf_s(fp, L"%lf %lf %lf\n", vYawPitchRoll.x, vYawPitchRoll.y, vYawPitchRoll.z);
			fwprintf_s(fp, L"%lf %lf %lf\n", vPos.x, vPos.y, vPos.z);

			// 트리 오브젝트의 경우 바람의 세기를 저장한다.
			if (e.first.find(L"tree") != std::wstring::npos)
			{
				TreeModel* pTemp = (TreeModel*)v.first->GetModel();
				AkF32 fWindTrunk = pTemp->GetWindTrunk();
				AkF32 fWindLeaves = pTemp->GetWindLeaves();
				fwprintf_s(fp, L"%lf \n", fWindTrunk);
				fwprintf_s(fp, L"%lf \n", fWindLeaves);
			}

			// 스톤 오브젝트의 경우 height map scale 을 저장한다.
		}
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
	//if (_pCSGDBHandle)
	//{
	//	_pCSGCube->DestoryDynamicVertexBuferHandle(_pCSGDBHandle);
	//	_pCSGDBHandle = nullptr;
	//}
	//if (_pCube)
	//{
	//	GeometryGenerator::DestroyGeometry(_pCube, 1);
	//	_pCube = nullptr;
	//}
	//if (_pCSGCube)
	//{
	//	_pCSGCube->Release();
	//	_pCSGCube = nullptr;
	//}

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

	//for (auto& e : _vecGameObj)
	//{
	//	delete e;
	//	e = nullptr;
	//}
	//_vecGameObj.clear();

	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			delete v.first;
			v.first = nullptr;
		}
		e.second.clear();
	}
	_mapGameObj.clear();

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

Ocean* EditorMap::CreateOcean()
{
	static AkI32 id = 0;
	Ocean* pOcean = new Ocean;
	pOcean->SetEditMode(AK_TRUE); // For GUI Control
	wcscpy_s(pOcean->Name, L"Ocean_");
	wchar_t wcBuf[32] = {};
	_itow_s(id, wcBuf, 10);
	wcscat_s(pOcean->Name, wcBuf);
	// _vecActFileNameList.push_back(pOcean->Name);
	id++;
	return pOcean;
}

Cloud* EditorMap::CreateCloud()
{
	static AkI32 id = 0;
	Cloud* pCloud = new Cloud;
	pCloud->SetEditMode(AK_TRUE); // For GUI Control
	wcscpy_s(pCloud->Name, L"Cloud_");
	wchar_t wcBuf[32] = {};
	_itow_s(id, wcBuf, 10);
	wcscat_s(pCloud->Name, wcBuf);
	// _vecActFileNameList.push_back(pCloud->Name);
	id++;
	return pCloud;
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
	// _vecGameObj.push_back(pObj);
	// _vecActFileNameList.push_back(wcFilePath);

	_mapGameObj[wcFilePath].push_back(std::make_pair(pObj, false));
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
	// Create Billboard Object.
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
	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			((ModelObject*)v.first)->RenderGUI();
		}
	}

	// Update light gizmo.
	for (auto& e : _vecLights)
	{
		e->RenderGUI();
	}
}

void EditorMap::UpdateObject()
{
	// 01. 삭제될 오브젝트인지 확인한다.
	for (auto iter0 = _mapGameObj.begin(); iter0 != _mapGameObj.end();)
	{
		for(auto iter1 = iter0->second.begin(); iter1 != iter0->second.end();)
		{
			// Delete 상태인지 확인.
			if (iter1->second)
			{
				if (iter1->first)
				{
					delete iter1->first;
					iter1->first = nullptr;

					iter1 = iter0->second.erase(iter1);
				}
				else
				{
					__debugbreak();
				}
			}
			else
			{
				iter1++;
			}
		}

		// 게임 오브젝트 자료구조의 데이터가 없다면 해당 맵의 요소는 지운다.
		if (iter0->second.empty())
		{
			iter0 = _mapGameObj.erase(iter0);
		}
		else
		{
			iter0++;
		}
	}

	// 02. 오브젝트 업데이트.
	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			v.first->Update();
		}
	}
}

void EditorMap::UpdateObjectDeleteState()
{
	for (auto& e : _mapGameObj)
	{
		for (auto& v : e.second)
		{
			AkBool bPicked = ((ModelObject*)v.first)->UseGizmo();
			if (bPicked)
			{
				if (KEY_DOWN(KEY_INPUT_DELETE))
				{
					// 오브젝트의 상태를 Delete 로 변경한다.
					v.second = true;
				}
			}
		}	
	}
}

