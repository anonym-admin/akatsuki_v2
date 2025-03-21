#include "pch.h"
#include "EditorModel.h"
#include "ModelExporter.h"
#include "ModelImporter.h"
#include "Camera.h"
#include "GeometryGenerator.h"
#include "ModelObject.h"

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
	GAssetManager->AddCubeMapTexture(IBL_FILE_PATH, L"PureSkyEnvHDR.dds", L"PureSkyDiffuseMDR.dds", L"PureSkySpecularHDR.dds", L"PureSkyBrdf.dds");

	AssetTextureContainer_t* pDiffuseHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::IRRADIANCE);
	AssetTextureContainer_t* pSpecularHDR = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::SPECULAR);
	AssetTextureContainer_t* pBrdf = GAssetManager->GetTextureContainer(ASSET_TEXTURE_TYPE::BRDF);

	GRenderer->BindIBLTexture(pDiffuseHDR->pTexHandle, pSpecularHDR->pTexHandle, pBrdf->pTexHandle);
	GRenderer->SetIBLStrength(0.25f);

	// Create Ground.
	AkU32 uMeshDataNum = 0;
	Vector2 vTexScale = Vector2(12.0f);
	MeshData_t* pSquare = GeometryGenerator::MakeSquare(&uMeshDataNum, 25.0f, &vTexScale);
	wcscpy_s(pSquare->wcAlbedoTextureFilename, L"../../assets/map/texture/Poliigon_TilesCeramicWhite_6956_BaseColor.dds");
	wcscpy_s(pSquare->wcAoTextureFilename, L"../../assets/map/texture/Poliigon_TilesCeramicWhite_6956_AmbientOcclusion.dds");
	wcscpy_s(pSquare->wcNormalTextureFilename, L"../../assets/map/texture/Poliigon_TilesCeramicWhite_6956_Normal.dds");
	wcscpy_s(pSquare->wcRoughnessTextureFilename, L"../../assets/map/texture/Poliigon_TilesCeramicWhite_6956_Roughness.dds");

	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissvie = Vector3(0.0f);
	_pGround = new Model(pSquare, uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissvie);
	
	Matrix mWorldRow = Matrix::CreateRotationX(DirectX::XM_PIDIV2);
	_pGround->UpdateWorldRow(&mWorldRow);

	GeometryGenerator::DestroyGeometry(pSquare, uMeshDataNum);

	return AK_TRUE;
}

AkBool EditorModel::BeginEditor()
{
	_pCamera->GetTransform()->SetPosition(&_vCamPos);
	_pCamera->GetTransform()->SetRotation(&_vCamYawPitchRoll);
	_bFPV = AK_TRUE;

	// For Draw Collider
	Collider::DRAW_COLLIDER = AK_TRUE;

	return AK_TRUE;
}

AkBool EditorModel::EndEditor()
{
	Collider::DRAW_COLLIDER = AK_FALSE;

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
	// Update
	UpdateControl();

	if(_bFPV)
	{
		_pCamera->Update();
	}

	if (_mapAnim[_CurCharacter] && _bPlayAnim)
	{
		_mapAnim[_CurCharacter]->Update();
	}

	// Final Update
	if(_pCollider)
	{
		if(_mapSkinnedModel.count(_CurCharacter))
		{
			Matrix mTargetWorld = _mapSkinnedModel[_CurCharacter]->GetWorldRow();

			_pCollider->GetTransform()->SetParent(&mTargetWorld);
		}

		_pCollider->Update();
	}

	// Gloabl Light.
	GRenderer->AddGlobalLight(&_vRadiance, &_vLightDir, AK_TRUE);
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
	std::string ModelType = "Model: " + ToString(_CurCharacter);
	ImGui::Text(ModelType.c_str());

	for (auto& e : _mapClipName[_CurCharacter])
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

	// Animation Blending
	ImGui::Begin("Animation Blending");

	// TODO

	ImGui::End();

	// Collider
	ImGui::Begin("Collider");
	ImGui::Checkbox("Attach Character", &_bAttachColliderToCharacter);
	const char* pColliderItems[] = { "Box", "Sphere", "Capsule" };
	if (ImGui::ListBox("Bone List", &_iSelectColliderTpye, pColliderItems, IM_ARRAYSIZE(pColliderItems)))
	{
		CreateCollider((COLLIDER_TYPE)_iSelectColliderTpye);
	}

	ImGui::End();

	// Lighting
	ImGui::Begin("Shadow");
	ImGui::SliderFloat3("irradiance", &_vRadiance.x, 0.0f, 5.0f);
	ImGui::SliderFloat3("light direction", &_vLightDir.x, -100.0f, 100.0f);
	ImGui::End();
}

void EditorModel::Render()
{
	UpdateGizmo();
	UpdateWeapon();

	// Render
	for (auto& e : _vecModel)
	{
		e->Render();
	}

	_pGround->Render();

	if(_pCollider)
	{
		_pCollider->Render();
	}

	if (_pCurBoneAnimation && _bBindAnim)
	{
		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			Matrix mTransform = _mapAnim[_CurCharacter]->GetBoneTrnasformAtID(_pCurBoneAnimation[i].iID).Transpose();

			_pCurBoneAnimation[i].mWorldRow = mTransform;

			_pCurBoneAnimation[i].Render();
		}
	}
}

void EditorModel::RenderShadow()
{
	for (auto& e : _vecModel)
	{
		e->RenderShadow();
	}

	_pGround->RenderShadow();
}

void EditorModel::CleanUp()
{
	if (_pCollider)
	{
		delete _pCollider;
		_pCollider = nullptr;
	}

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

	if (_pGround)
	{
		delete _pGround;
		_pGround = nullptr;
	}

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
	Matrix mWorldRow = Matrix::CreateTranslation(Vector3(0.0f, 0.5f, 0.0f));
	pModel->UpdateWorldRow(&mWorldRow);

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
	_CurCharacter = ModelName;

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

void EditorModel::CreateCollider(COLLIDER_TYPE eType)
{
	if (!_bAttachColliderToCharacter)
	{

	}
	else
	{ 
		switch (eType)
		{
		case COLLIDER_TYPE::BOX:

			break;
		case COLLIDER_TYPE::SPHERE:

			break;
		case COLLIDER_TYPE::CAPSULE:
			_pCollider = new CapsuleCollider(nullptr);
			break;
		}
	}
}

void EditorModel::BindAnimation()
{
	if (!_mapSkinnedModel.count(_CurCharacter) || !_mapAnim.count(_CurCharacter))
	{
		return;
	}

	if (_bBindAnim)
	{
		_mapSkinnedModel[_CurCharacter]->BindAnimation(_mapAnim[_CurCharacter]);
	}
	else
	{
		_mapSkinnedModel[_CurCharacter]->BindAnimation(nullptr);
	}
}

void EditorModel::AttachBone()
{
	if (!_bAttachBone)
	{
		return;
	}

	SkinnedModel* pOwner = _mapSkinnedModel[_CurCharacter];
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
			_pCurBoneAnimation[i].mBoneTransform = _mapAnim[_CurCharacter]->GetBoneTrnasformAtID(i).Transpose();
			_pCurBoneAnimation[i].RenderGUI();
		}
	}

	switch (_iSelectColliderTpye)
	{
	case 0:
	{

	}
	break;
	case 1:
	{

	}
	break;
	case 2:
	{
		_pCollider->RenderGUI();
	}
	break;
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
		_mapAnim[_CurCharacter]->PlayClip(wcClip.c_str(), ANIM_CLIP_STATE::LOOP, fSpeed, fBlendTime);
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

	SkinnedModel* pOwner = _mapSkinnedModel[_CurCharacter];
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
		wprintf_s(L"[Error] Please Select Model.\n");
		return;
	}

	if (!_pCurBoneAnimation)
	{
		wprintf_s(L"[Error] Please Select Animation clip.\n");
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
		wprintf_s(L"[Error] Please Select Bone.\n");
		return;
	}

	Matrix mTransform = Matrix();

	if (_bBindAnim)
	{
		mTransform = _mAttachMatrix * _mapAnim[_CurCharacter]->GetBoneTrnasformAtID(pTargetBone->iID).Transpose();

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

