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
	GAssetManager->AddCubeMapTexture(IBL_FILE_PATH, L"SampleEnvHDR.dds", L"SampleDiffuseMDR.dds", L"SampleSpecularHDR.dds", L"SampleBrdf.dds");

	AssetTextureContainer_t* pDiffuseHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::IRRADIANCE);
	AssetTextureContainer_t* pSpecularHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::SPECULAR);
	AssetTextureContainer_t* pBrdf = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::BRDF);

	GRenderer->BindIBLTexture(pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle, pBrdf->pTexHandle);
	GRenderer->SetIBLStrength(0.5f);

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

	if (_bFPV)
	{
		_pCamera->Update();
	}

	if (_mapAnim[_CurModel] && _bPlayAnim)
	{
		_mapAnim[_CurModel]->Update();
	}
}

void EditorModel::RenderGUI()
{
	_pCamera->RenderGUI();

	ImGui::Begin("[Model Editor]");
	ImGui::Checkbox("FBV", &_bFPV);
	if (ImGui::Checkbox("Bind Anim", &_bBindAnim))
	{
		BindAnimation();
	}

	IGFD::FileDialogConfig tConfig = {};
	tConfig.filePathName = "../../assets/model_new/origin/";
	const char* pItems0[] = { "Model", "Animation" };
	if (ImGui::Combo("Export Data Type", &_iExportType, pItems0, IM_ARRAYSIZE(pItems0)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ExportKey", "Choose File", ".fbx,.gltf,.obj", tConfig);
	}

	tConfig.filePathName = "../../assets/model_new/";
	const char* pItems1[] = { "Model", "Animation" };
	if (ImGui::Combo("Modify Data Type", &_iModifyType, pItems1, IM_ARRAYSIZE(pItems1)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ModifyKey", "Choose File", ".anim,.mesh", tConfig);
	}

	tConfig.filePathName = "../../assets/model_new/";
	const char* pItems2[] = { "Model", "SkinnedModel", "Animation" };
	if (ImGui::Combo("Import Data Type", &_iImportType, pItems2, IM_ARRAYSIZE(pItems2)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ImportKey", "Choose File", ".mesh,.anim", tConfig);
	}

	UpdateFileDialog();
	ImGui::End();

	// Clip list.
	ImGui::Begin("[Clips]");
	std::string ModelType = "Model: " + ToString(_CurModel);
	ImGui::Text(ModelType.c_str());

	for (auto& e : _mapClipName[_CurModel])
	{
		if (ImGui::Button(ToString(e).c_str()))
		{
			_bPlayAnim = !_bPlayAnim;

			SetAnimation(e, _fAnimSpeed, _fBlendTime);

			_pCurBoneAnimation = _mapClip[_CurClip]->pBoneAnimationList;
		}
	}
	if (_bPlayAnim)
	{
		ImGui::Text("Playing Anim");
	}
	ImGui::End();

	// Animation Info.
	ImGui::Begin("[Amimation Info]");
	ImGui::InputFloat("Anim Speed", &_fAnimSpeed);
	ImGui::InputFloat("Blend Time:", &_fBlendTime);

	ImGui::ListBox("Bone List", &_iCurSelectedBoneID, _ppBoneName, _uBoneNum);
	if (-1 != _iCurSelectedBoneID)
	{
		ImGui::Text("Bone ID: %d", _iCurSelectedBoneID);
	}
	else
	{
		ImGui::Text("Bone ID: not select");
	}
	if (_pCurBoneAnimation)
	{
		if (_iPrevSelectedBoneID >= 0 && _iCurSelectedBoneID >= 0)
		{
			_pCurBoneAnimation[_iPrevSelectedBoneID].bPick = AK_FALSE;
			_pCurBoneAnimation[_iCurSelectedBoneID].bPick = AK_TRUE;

			_iPrevSelectedBoneID = _iCurSelectedBoneID;
		}

	}
	if (ImGui::Checkbox("Attach Bone", &_bAttachBone))
	{
		AttachBone();
	}
	ImGui::Checkbox("Modify Weapon Matrix", &_bModifyWeaponTransform);


	ImGui::End();
}

void EditorModel::Render()
{
	UpdateGizmo();
	UpdateWeapon();

	for (auto& e : _vecModel)
	{
		e->Render();
	}

	if (_pCurBoneAnimation && _bBindAnim)
	{
		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			Matrix mTransform = _mapAnim[_CurModel]->GetBoneTrnasformAtID(_pCurBoneAnimation[i].iID).Transpose();

			_pCurBoneAnimation[i].mWorldRow = mTransform;

			_pCurBoneAnimation[i].Render();
		}
	}
}

void EditorModel::RenderShadow()
{

}

void EditorModel::CleanUp()
{
	_mapClipName.clear();
	_mapSkinnedModel.clear();

	for (auto& e : _mapClip)
	{
		BoneAnimation_t* pBoneList = e.second->pBoneAnimationList;
		if (pBoneList)
		{
			for (AkU32 i = 0; i < _uBoneNum; i++)
			{
				if (pBoneList[i].pLineObj)
				{
					pBoneList[i].pLineObj->Release();
					pBoneList[i].pLineObj = nullptr;
				}
			}
		}
	}

	if (!_bUseAnim)
	{
		delete[] _pBoneOffsetMatrixList;
		_pBoneOffsetMatrixList = nullptr;

		delete[] _pBoneHierarchyList;
		_pBoneHierarchyList = nullptr;

		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			free(_ppBoneName[i]);
			_ppBoneName[i] = nullptr;
		}

		free(_ppBoneName);
		_ppBoneName = nullptr;
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

void EditorModel::ModifyAnimation(const std::wstring& wcName, const std::wstring& wcClip)
{
	using namespace std;

#ifdef _DEBUG
	wprintf_s(L"\n[Modify Clips Start]\n");
#endif

	wstring wcPath = L"../../assets/model_new/animation/" + wcName + L"/" + wcClip + L".anim";

	ofstream fout;
	fout.open(ToString(wcPath).c_str());

	wstring wcClipName = wcClip + L".anim";

	// write animation clip
	fout << "========AnimationClip========" << endl;
	fout << "AnimationClipCount: " << 1 << endl;
	for (AkU32 i = 0; i < 1; i++)
	{
		fout << "AnimationClip" << i << ": " << ToString(GetFileName(wcPath)) << endl;
		fout << "Duration: " << _mapClip[wcClipName]->uDuration << endl;
		fout << "TicksPerSecond: " << _mapClip[wcClipName]->uTickPerSecond << endl;
		fout << "{" << endl;
		for (AkU32 j = 0; j < _mapClip[wcClipName]->uNumBoneAnimation; j++)
		{
			fout << "\tBone" << j << " " << "KeyFrame: " << _mapClip[wcClipName]->pBoneAnimationList[j].uNumKeyFrame << endl;
			fout << "\t{" << endl;
			for (AkU32 k = 0; k < _mapClip[wcClipName]->pBoneAnimationList[j].uNumKeyFrame; k++)
			{
				fout << "\t\tTime: " << k << endl;
				fout << "\t\t\tPos: " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].vPos.x << " " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].vPos.y << " " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].vPos.z << endl;
				fout << "\t\t\tScale: " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].vScale.x << " " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].vScale.y << " " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].vScale.z << endl;
				fout << "\t\t\tQuat: " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].qRot.x << " " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].qRot.y << " " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].qRot.z << " " << _mapClip[wcClipName]->pBoneAnimationList[j].pKeyFrameList[k].qRot.w << endl;
			}
			fout << "\t}" << endl;
		}
		fout << "}" << endl;
	}

	if (fout.is_open())
	{
		fout.close();
	}

#ifdef _DEBUG
	wprintf_s(L"\n[Modify Clips End]\n");
#endif
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
		_ppBoneName = _pImporter->GetBoneName();
		_uBoneNum = _pImporter->GetBoneNum();

		Vector3 vAlbedo = Vector3(1.0f);
		Vector3 vEmissive = Vector3(0.0f);
		pModel = new SkinnedModel(pMeshData, uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissive);

		_mapSkinnedModel[GetFileNmaeExcludeExt(wcFilename)] = (SkinnedModel*)pModel;
	}

	wcscpy_s(pModel->Name, GetFileNmaeExcludeExt(wcFilename).c_str());

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
	tMeshDataContainer.ppBoneName = _ppBoneName;
	tMeshDataContainer.uBoneNum = _uBoneNum;

	if (!_mapAnim.count(ModelName))
	{
		Animation* pAnim = new Animation(&tMeshDataContainer, wcClip.c_str(), 32); // 초기화를 위한 IDLE Clip Name Parameter로 전달
		_mapAnim[ModelName] = pAnim;
	}

	if (!_mapClipName[ModelName].empty())
	{
		for (auto& e : _mapClipName[ModelName])
		{
			if (e == wcClip)
			{
				return;
			}
		}
	}

	_mapClip[wcClip] = _mapAnim[ModelName]->ReadClip(wcPath.c_str(), wcClip.c_str());
	_mapClipName[ModelName].push_back(wcClip);

	_bUseAnim = AK_TRUE;
	_CurModel = ModelName;

	{
		_pCurBoneAnimation = _mapClip[wcClip]->pBoneAnimationList;

		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			AkI32 iParentID = (_pBoneHierarchyList[i] == -1) ? 0 : _pBoneHierarchyList[i];

			_pCurBoneAnimation[i].iID = (AkI32)iParentID;
			strcpy_s(_pCurBoneAnimation[i].pName, _ppBoneName[i]);

			_pCurBoneAnimation[i].vStart = Vector3::Transform(Vector3(0.0f), _pBoneOffsetMatrixList[i].Invert() * _mDefaultMatrix);

			_pCurBoneAnimation[i].vEnd = Vector3::Transform(Vector3(0.0f), _pBoneOffsetMatrixList[iParentID].Invert() * _mDefaultMatrix);

			VertexColor_t tStart = { _pCurBoneAnimation[i].vStart, _pCurBoneAnimation[i].vColor };
			VertexColor_t tEnd = { _pCurBoneAnimation[i].vEnd, _pCurBoneAnimation[i].vColor };

			_pCurBoneAnimation[i].pLineObj = GRenderer->CreateLineObject();
			_pCurBoneAnimation[i].pLineObj->CreateLineBuffer(&tStart, &tEnd);
		}
	}
}

void EditorModel::BindAnimation()
{
	if (!_mapSkinnedModel.count(_CurModel) || !_mapAnim.count(_CurModel))
	{
		return;
	}

	if (_bBindAnim)
	{
		_mapSkinnedModel[_CurModel]->BindAnimation(_mapAnim[_CurModel]);
	}
	else
	{
		_mapSkinnedModel[_CurModel]->BindAnimation(nullptr);
	}
}

void EditorModel::AttachBone()
{
	if (!_bAttachBone)
	{
		return;
	}

	SkinnedModel* pOwner = _mapSkinnedModel[_CurModel];
	Model* pTargetModel = nullptr;
	for (auto& e : _vecModel)
	{
		if (e->IsPick())
		{
			pTargetModel = e;
			break;
		}
	}

	if (!pTargetModel)
	{
		return;
	}

	if (!_pCurBoneAnimation)
	{
		wprintf_s(L"[Error] Do not Select Animation clip.\n");
		return;
	}

	BoneAnimation_t* pTargetBone = nullptr;
	for (AkU32 i = 0; i < _uBoneNum; i++)
	{
		if (_pCurBoneAnimation[i].bPick)
		{
			pTargetBone = _pCurBoneAnimation + i;
			break;
		}
	}

	if (!pTargetBone)
	{
		return;
	}

	Matrix mTargetTransform = Matrix();
	mTargetTransform *= Matrix::CreateTranslation(pTargetBone->vStart);
	pTargetModel->UpdateWorldRow(&mTargetTransform);

	_mAttachMatrix = mTargetTransform;
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

	if (ImGuiFileDialog::Instance()->Display("ModifyKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			switch (_iModifyType)
			{
			case 0:

				break;
			case 1:
				ModifyAnimation(ToWString(GetFileName(FilePath)), ToWString(GetFileNmaeExcludeExt(GetFileName(FileName))));
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

void EditorModel::UpdateGizmo()
{
	for (auto& e : _vecModel)
	{
		e->RenderGUI();
	}

	if (_pCurBoneAnimation)
	{
		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			_pCurBoneAnimation[i].mBoneTransform = _mapAnim[_CurModel]->GetBoneTrnasformAtID(i).Transpose();
			_pCurBoneAnimation[i].RenderGUI();
		}
	}
}

void EditorModel::UpdateControl()
{
	if (KEY_DOWN(KEY_INPUT_F))
	{
		_bFPV = !_bFPV;
	}
}

void EditorModel::SetAnimation(const std::wstring& wcClip, AkF32 fSpeed, AkF32 fBlendTime)
{
	if (_CurClip != wcClip || _fPrevAnimSpeed != fSpeed || _fPrevBlendTime != fBlendTime)
	{
		_mapAnim[_CurModel]->PlayClip(wcClip.c_str(), ANIM_CLIP_STATE::LOOP, fSpeed, fBlendTime);
		_CurClip = wcClip;
		_fPrevAnimSpeed = fSpeed;
		_fPrevBlendTime = fBlendTime;
	}
}

void EditorModel::UpdateWeapon()
{
	if (!_bAttachBone)
	{
		return;
	}

	SkinnedModel* pOwner = _mapSkinnedModel[_CurModel];
	Model* pTargetModel = nullptr;
	for (auto& e : _vecModel)
	{
		if (e->IsPick())
		{
			if (pOwner == pTargetModel)
			{
				continue;
			}

			pTargetModel = e;
			break;
		}
	}

	if (!pTargetModel)
	{
		return;
	}

	if (!_pCurBoneAnimation)
	{
		wprintf_s(L"[Error] Do not Select Animation clip.\n");
		return;
	}

	BoneAnimation_t* pTargetBone = nullptr;
	for (AkU32 i = 0; i < _uBoneNum; i++)
	{
		if (_pCurBoneAnimation[i].bPick)
		{
			pTargetBone = _pCurBoneAnimation + i;
			break;
		}
	}

	if (!pTargetBone)
	{
		return;
	}

	Matrix mTransform = Matrix();

	if (_bBindAnim)
	{
		mTransform = _mAttachMatrix * _mapAnim[_CurModel]->GetBoneTrnasformAtID(pTargetBone->iID).Transpose();

		pTargetModel->UpdateWorldRow(&mTransform);
	}
	else
	{
		if (!_bModifyWeaponTransform)
		{
			mTransform = _mAttachMatrix;

			pTargetModel->UpdateWorldRow(&mTransform);
		}
		else
		{
			mTransform = pTargetModel->GetWorldRow();
			_mAttachMatrix = mTransform;
		}
	}
}

