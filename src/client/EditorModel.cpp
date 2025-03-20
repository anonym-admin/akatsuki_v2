#include "pch.h"
#include "EditorModel.h"
#include "ModelExporter.h"
#include "ModelImporter.h"
#include "Camera.h"
#include "GeometryGenerator.h"

EditorModel::EditorModel()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

EditorModel::~EditorModel()
{
	CleanUp();
}

AkBool EditorModel::Initialize()
{
	_pCamera = new Camera(&_vCamPos, &_vCamYawPitchRoll);
	_pCamera->Mode = CAMERA_MODE::EDITOR;

	// Bind IBL Texture For PBR.
	GAssetManager->AddCubeMapTexture(IBL_FILE_PATH, L"PureSkyEnvHDR.dds", L"PureSkyDiffuseHDR.dds", L"PureSkySpecularHDR.dds", L"PureSkyBrdf.dds");

	AssetTextureContainer_t* pDiffuseHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::IRRADIANCE);
	AssetTextureContainer_t* pSpecularHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::SPECULAR);
	AssetTextureContainer_t* pBrdf = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::BRDF);

	GRenderer->BindIBLTexture(pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle, pBrdf->pTexHandle);
	GRenderer->SetIBLStrength(0.25f);

	return AK_TRUE;
}

AkBool EditorModel::BeginEditor()
{
	_pCamera->GetTransform()->SetPosition(&_vCamPos);
	_pCamera->GetTransform()->SetRotation(&_vCamYawPitchRoll);
	_bFPV = AK_FALSE;

	return AK_TRUE;
}

AkBool EditorModel::EndEditor()
{
	for (auto& e : _vecModel)
	{
		delete e;
		e = nullptr;
	}
	_vecModel.clear();

	return AK_TRUE;
}

void EditorModel::Update()
{
	UpdateControl();

	if(_bFPV)
	{
		_pCamera->Update();
	}

	if(_mapAnim[_CurModel])
	{
		_mapAnim[_CurModel]->Update();
	}
}

void EditorModel::FinalUpdate()
{
	_pCamera->UpdateEditor();

	ImGui::Begin("[Model Editor]");
	ImGui::Checkbox("FBV", &_bFPV);
	ImGui::Checkbox("Bind Anim", &_bBindAnim);

	BindAnimation();

	IGFD::FileDialogConfig tConfig = {};
	tConfig.filePathName = "../../assets/model_new/origin/";
	const char* pItems0[] = { "Model", "Animation" };
	if (ImGui::Combo("Export Data Type", &_iExportType, pItems0, IM_ARRAYSIZE(pItems0)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ExportKey", "Choose File", ".fbx,.gltf,.obj", tConfig);
	}

	tConfig.filePathName = "../../assets/model_new/";
	const char* pItems1[] = { "Model", "SkinnedModel", "Animation" };
	if (ImGui::Combo("Import Data Type", &_iImportType, pItems1, IM_ARRAYSIZE(pItems1)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ImportKey", "Choose File", ".mesh,.anim", tConfig);
	}

	UpdateFileDialog();

	ImGui::End();

	ImGui::Begin("[Clips]");
	ImGui::Text(ToString(_CurModel).c_str());

	if (ImGui::Button("Push"))
		SetAnimation(_CurClip);

	ImGui::End();
}

void EditorModel::Render()
{
	for (auto& e : _vecModel)
	{
		e->Render();
	}
}

void EditorModel::RenderShadow()
{

}

void EditorModel::CleanUp()
{
	_mapClipName.clear();
	_mapSkinnedModel.clear();

	if (!_mapAnim.size())
	{
		delete[] _pBoneOffsetMatrixList;
		_pBoneOffsetMatrixList = nullptr;

		delete[] _pBoneHierarchyList;
		_pBoneHierarchyList = nullptr;
	}

	for (auto& e : _mapAnim)
	{
		delete e.second;
		e.second = nullptr;
	}
	_mapAnim.clear();

	for (auto& e : _vecModel)
	{
		delete e;
		e = nullptr;
	}
	_vecModel.clear();

	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}
}

void EditorModel::Load(const std::wstring& wcFilePath)
{
}

void EditorModel::Save(const std::wstring& wcFilePath)
{
}

void EditorModel::ExportMesh(const std::wstring& wcName, const std::wstring& wcExt)
{
	_pExporter = new ModelExporter(ToString(L"../../assets/model_new/origin/models/" + wcName + L"." + wcExt));
	_pExporter->ExportMesh();
	delete _pExporter;
}

void EditorModel::ExportAnimation(const std::wstring& wcName, const std::wstring& wcClip)
{
	_pExporter = new ModelExporter(ToString(L"../../assets/model_new/origin/animations/" + wcName + L"/" + wcClip + L".fbx"));
	_pExporter->ExportClip();
	delete _pExporter;
}

void EditorModel::CreateModel(const std::wstring& wcBasePath, const std::wstring& wcFilename)
{
	Model* pModel = nullptr;
	MeshData_t* pMeshData = nullptr;
	AkU32 uMeshDataNum = 0;

	_pImporter = new ModelImporter;

	if (0 == _iImportType)
	{
		_pImporter->Load(wcBasePath.c_str(), wcFilename.c_str(), AK_FALSE);

		pMeshData = _pImporter->GetMeshData();
		uMeshDataNum = _pImporter->GetMeshDataNum();

		GeometryGenerator::NormalizeMeshData(pMeshData, uMeshDataNum, 1.0f, AK_FALSE, nullptr);

		Vector3 vAlbedo = Vector3(1.0f);
		Vector3 vEmissive = Vector3(0.0f);
		
		pModel = new Model(pMeshData, uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissive);
	}
	else if (1 == _iImportType)
	{
		_pImporter->Load(wcBasePath.c_str(), wcFilename.c_str(), AK_TRUE);

		pMeshData = _pImporter->GetMeshData();
		uMeshDataNum = _pImporter->GetMeshDataNum();

		GeometryGenerator::NormalizeMeshData(pMeshData, uMeshDataNum, 1.0f, AK_TRUE, &_mDefaultMatrix);
		_pBoneOffsetMatrixList = _pImporter->GetBoneOffsetTransformList();
		_pBoneHierarchyList = _pImporter->GetBoneHierarchyList();
		_uBoneNum = _pImporter->GetBoneNum();

		Vector3 vAlbedo = Vector3(1.0f);
		Vector3 vEmissive = Vector3(0.0f);
		pModel = new SkinnedModel(pMeshData, uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissive);

		_mapSkinnedModel[GetFileNmaeExcludeExt(wcFilename)] = (SkinnedModel*)pModel;
	}

	_vecModel.push_back(pModel);

	GeometryGenerator::DestroyGeometry(pMeshData, uMeshDataNum);

	delete _pImporter;
}

void EditorModel::CreateClip(const std::wstring& wcPath, const std::wstring& wcClip)
{
	std::wstring ModelName = GetCurrentFolder(wcPath);

	if (!_mapSkinnedModel.count(ModelName))
	{
		return;
	}

	AssetMeshDataContainer_t tMeshDataContainer = {};
	tMeshDataContainer.pBoneOffsetMatrixList = _pBoneOffsetMatrixList;
	tMeshDataContainer.pBoneHierarchyList = _pBoneHierarchyList;
	tMeshDataContainer.mDefaultMat = _mDefaultMatrix;
	tMeshDataContainer.uBoneNum = _uBoneNum;

	if(!_mapAnim.count(ModelName))
	{
		Animation* pAnim = new Animation(&tMeshDataContainer, wcClip.c_str(), 32); // 초기화를 위한 IDLE Clip Name Parameter로 전달
		_mapAnim[ModelName] = pAnim;
	}

	if(!_mapClipName.count(ModelName))
	{
		_mapAnim[ModelName]->ReadClip(wcPath.c_str(), wcClip.c_str());
		for (auto& e : _mapClipName[ModelName])
		{
			if (e != wcClip)
			{
				_mapClipName[ModelName].push_back(wcClip);
			}
		}
	}

	_CurModel = ModelName;
	_CurClip = wcClip;
}

void EditorModel::BindAnimation()
{
	if (!_bBindAnim)
	{
		return;
	}

	if (!_mapSkinnedModel.count(_CurModel) || !_mapAnim.count(_CurModel))
	{
		return;
	}
	
	_mapSkinnedModel[_CurModel]->BindAnimation(_mapAnim[_CurModel]);
}

void EditorModel::UpdateFileDialog()
{
	if (ImGuiFileDialog::Instance()->Display("ExportKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			switch (_iExportType)
			{
			case 0:
				ExportMesh(ToWString(GetFileNmaeExcludeExt(GetFileName(FileName))), ToWString(GetFileExtension(FileName)));
				break;
			case 1:
				ExportAnimation(ToWString(GetFileName(FilePath)), ToWString(GetFileNmaeExcludeExt(GetFileName(FileName))));
				break;
			}
		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("ImportKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

			switch (_iImportType)
			{
			case 0:
				CreateModel(ToWString(FilePath), ToWString(GetFileName(FileName)));
				break;
			case 1:
				CreateModel(ToWString(FilePath), ToWString(GetFileName(FileName)));
				break;
			case 2:
				CreateClip(ToWString(FilePath), ToWString(GetFileName(FileName)));
				break;
			}
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void EditorModel::UpdateControl()
{
	if (KEY_DOWN(KEY_INPUT_F))
	{
		_bFPV = !_bFPV;
	}
}

void EditorModel::SetAnimation(const std::wstring& wcClip, AkF32 fSpeed)
{
	if (_CurClip != wcClip)
	{
		_mapAnim[_CurModel]->PlayClip(wcClip.c_str(), ANIM_CLIP_STATE::LOOP, fSpeed);
		_CurClip = wcClip;
	}
}

