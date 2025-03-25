#include "pch.h"
#include "EditorMap.h"
#include "Camera.h"
#include "Terrain.h"
#include "ModelObject.h"

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

	// Update
	for (auto& e : _vecGameObj)
	{
		e->Update();
	}

	// Final Update
	for (auto& e : _vecGameObj)
	{
		e->FinalUpdate();
	}
}

void EditorMap::Render()
{
	UpdateGizmo();

	_pTerrainEdit->Render();

	for (auto& e : _vecGameObj)
	{
		e->Render();
	}
}

void EditorMap::RenderShadow()
{
	for (auto& e : _vecGameObj)
	{
		e->RenderShadow();
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

	const char* pTex[] = { "Albedo", "Second", "Third" };
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
		_bSave = AK_TRUE;
		_iSelectMode = 3;
		ImGuiFileDialog::Instance()->OpenDialog("ImportActorData", "Choose File", ".act", tConfig);
	}
	ImGui::End();

	UpdateFileDialog();
}

void EditorMap::Load(const std::wstring& wcFilePath)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFilePath.c_str(), L"wt");
	if (!fp)
	{
		__debugbreak();
	}






	if (fp)
	{
		fclose(fp);
		fp = nullptr;
	}
}

void EditorMap::Save(const std::wstring& wcFilePath)
{
}

void EditorMap::CleanUp()
{
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

void EditorMap::ExportMap(const std::wstring& wcFilePath)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFilePath.c_str(), L"wt");
	if (!fp)
	{
		__debugbreak();
	}

	// 01. Texture name
	fwprintf_s(fp, L"%s\n", _wcAlbedoFilename.empty() ? L"None" : _wcAlbedoFilename.c_str());
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
}

void EditorMap::ImportActor(const std::wstring& wcFilePath)
{
	ModelObject* pObj = new ModelObject(wcFilePath.c_str());
	pObj->SetEditMode(AK_TRUE);
	_vecGameObj.push_back(pObj);
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
						_pTerrainEdit->SetTextures((_wcFilePath + _wcFileName).c_str(), nullptr, nullptr);
					}
					else if (1 == _iTextureType) // Second
					{
						_wcSecondFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, (_wcFilePath + _wcFileName).c_str(), nullptr);
					}
					else if (2 == _iTextureType) // Third
					{
						_wcThirdFilename = _wcFilePath + _wcFileName;
						_pTerrainEdit->SetTextures(nullptr, nullptr, (_wcFilePath + _wcFileName).c_str());
					}
					break;
				}
				case 3:
				{
					ImportMap((_wcFilePath + _wcFileName).c_str());
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
					ExportMap((_wcFilePath + _wcFileName).c_str());
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
}

void EditorMap::UpdateGizmo()
{
	for (auto& e : _vecGameObj)
	{
		((ModelObject*)e)->RenderGUI();
	}
}

