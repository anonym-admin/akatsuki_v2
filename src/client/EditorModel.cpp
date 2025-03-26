#include "pch.h"
#include "EditorModel.h"
#include "ModelExporter.h"
#include "ModelImporter.h"
#include "Camera.h"
#include "GeometryGenerator.h"
#include "ModelObject.h"

/*
=============
Model Editor
=============
*/

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

	AssetTextureContainer_t* pDiffuseHDR = GAssetManager->GetTexture(L"PureSkyDiffuseHDR.dds");
	AssetTextureContainer_t* pSpecularHDR = GAssetManager->GetTexture(L"PureSkySpecularHDR.dds");
	AssetTextureContainer_t* pBrdf = GAssetManager->GetTexture(L"PureSkyBrdf.dds");

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

	// 모든 모델 삭제.
	for (auto& e : _vecModel)
	{
		delete e;
		e = nullptr;
	}
	_vecModel.clear();

	// 나머지 오블젝트 삭제??

	return AK_TRUE;
}

void EditorModel::Update()
{
	// Update
	UpdateControl();

	if (_bFPV)
	{
		_pCamera->Update();
	}

	if (!_mapAnim.empty() && _bPlayAnim)
	{
		_mapAnim[_CurCharacter]->Update();
	}

	// Final Update
	for (auto& v : _mapColliders)
	{
		for (auto& e : v.second)
		{
			if (_mapBasicModel[v.first])
			{
				Matrix mWorldRow = _mapBasicModel[v.first]->GetWorldRow();

				e->GetTransform()->SetParent(&mWorldRow);
			}

			if (_mapSkinnedModel[v.first])
			{
				Matrix mWorldRow = _mapSkinnedModel[v.first]->GetWorldRow();

				e->GetTransform()->SetParent(&mWorldRow);
			}

			e->Update();
		}
	}

	// Gloabl Light.
	GRenderer->AddGlobalLight(&_vRadiance, &_vLightDir, AK_TRUE);

	// Delete.
	DeleteProcess();
}

void EditorModel::RenderGUI()
{
	_pCamera->RenderGUI();

	ImGui::Begin("[Model Editor]");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", DT, FPS);
	ImGui::Checkbox("FBV", &_bFPV);
	if (ImGui::Checkbox("Bind Anim", &_bBindAnim))
	{
		BindAnimation();
	}
	ImGui::Checkbox("Render Bone", &_bRenderBone);
	ImGui::Checkbox("Render Character", &_bRenderCharacter);

	// File Import & Export
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

	if (ImGui::Button("Convert To DDS File"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("DDSKey", "Choose File", ".jpg,.png,.dds", tConfig);
	}

	tConfig.filePathName = "../../assets/model_new/textures/";
	const char* pItems3[] = { "Albedo", "Emissive", "Height", "Normal", "Metallic", "Roughness", "AO" };
	if (ImGui::Combo("Import Texture Type", &_iTextureType, pItems3, IM_ARRAYSIZE(pItems3)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("TextureKey", "Choose File", ".jpg,.png,.dds", tConfig);
	}

	if (ImGui::Button("Create Sphere Model"))
	{
		AkU32 uMeshDataNum = 0;
		MeshData_t* pMeshData = GeometryGenerator::MakeSphere(&uMeshDataNum, _fGeoSphereRadius, _uGeoSphereSlice, _uGeoSphereStack);
		Vector3 vAlbedo = Vector3(0.5f);
		Vector3 vEmissive = Vector3(0.0f);
		Model* pSphere = new Model(pMeshData, uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissive);

		_vecModel.push_back(pSphere);

		GeometryGenerator::DestroyGeometry(pMeshData, uMeshDataNum);
	}
	if (ImGui::Button("Create Cube Model"))
	{
		AkU32 uMeshDataNum = 0;
		MeshData_t* pMeshData = GeometryGenerator::MakeCube(&uMeshDataNum);
		Vector3 vAlbedo = Vector3(0.5f);
		Vector3 vEmissive = Vector3(0.0f);
		Model* pCube = new Model(pMeshData, uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissive);

		_vecModel.push_back(pCube);

		GeometryGenerator::DestroyGeometry(pMeshData, uMeshDataNum);
	}

	tConfig.filePathName = "../../assets/model_new/mesh/";
	const char* pItems4[] = { "Sphere", "Cube" };
	if (ImGui::Combo("Save Geometry Model Type", &_iGeometryType, pItems4, IM_ARRAYSIZE(pItems4)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("GeometryModelKey", "Choose File", ".mesh", tConfig);
	}

	// Save Actor Data
	tConfig.filePathName = "../../assets/";
	const char* pItems5[] = { "Actor Data" };
	if (ImGui::Combo("Save Actor Data Type", &_iSaveActorDataType, pItems5, IM_ARRAYSIZE(pItems5)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("SaveActorDataKey", "Choose File", ".mesh,.anim", tConfig);
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

	// Animation Combine
	static std::wstring wcAttachClipName = L"";
	ImGui::Begin("Animation Combine");
	{
		for (auto& e : _mapClipName[_CurCharacter])
		{
			if (ImGui::Button(ToString(e).c_str()))
			{
				_pAttachBoneAnimation = _mapClip[e]->pBoneAnimationList;
				wcAttachClipName = e;
			}
		}
	}

	_CurClip.empty() ? ImGui::Text("Body Cur Clip: Not Select") : ImGui::Text(ToString(_CurClip).c_str());
	wcAttachClipName.empty() ? ImGui::Text("Leg Attach Clip: Not Select") : ImGui::Text(ToString(wcAttachClipName).c_str());
	AkBool bComplete = AK_FALSE;
	if (ImGui::Button("Bake Animation"))
	{
		if (_pCombineAnimationClip)
		{
			delete _pCombineAnimationClip;
			_pCombineAnimationClip = nullptr;
		}

		_pCombineBoneAnimation = new BoneAnimation_t[_uBoneNum];

		if (_pAttachBoneAnimation && _pCurBoneAnimation)
		{
			// Hip
			AkU32 uKeyFrameNum = _pAttachBoneAnimation[0].uNumKeyFrame;
			_pCombineBoneAnimation[0].uNumKeyFrame = uKeyFrameNum;
			_pCombineBoneAnimation[0].pKeyFrameList = new KeyFrame_t[uKeyFrameNum];
			memcpy(_pCombineBoneAnimation[0].pKeyFrameList, _pAttachBoneAnimation[0].pKeyFrameList, sizeof(KeyFrame_t) * uKeyFrameNum);

			// Body
			for (AkU32 i = 1; i <= 58; i++) // 일반화 필요 => 모델 마다 본의 인덱스가 다름.
			{
				AkU32 uKeyFrameNum = _pCurBoneAnimation[i].uNumKeyFrame;
				_pCombineBoneAnimation[i].uNumKeyFrame = uKeyFrameNum;
				_pCombineBoneAnimation[i].pKeyFrameList = new KeyFrame_t[uKeyFrameNum];
				memcpy(_pCombineBoneAnimation[i].pKeyFrameList, _pCurBoneAnimation[i].pKeyFrameList, sizeof(KeyFrame_t) * uKeyFrameNum);
			}

			// Leg
			for (AkU32 i = 59; i <= _uBoneNum - 1; i++)
			{
				AkU32 uKeyFrameNum = _pAttachBoneAnimation[i].uNumKeyFrame;
				_pCombineBoneAnimation[i].uNumKeyFrame = uKeyFrameNum;
				_pCombineBoneAnimation[i].pKeyFrameList = new KeyFrame_t[uKeyFrameNum];
				memcpy(_pCombineBoneAnimation[i].pKeyFrameList, _pAttachBoneAnimation[i].pKeyFrameList, sizeof(KeyFrame_t) * uKeyFrameNum);
			}

			// Save File.
			_pCombineAnimationClip = new AnimationClip_t;
			_pCombineAnimationClip->pBoneAnimationList = _pCombineBoneAnimation;
			_pCombineAnimationClip->uNumBoneAnimation = _uBoneNum;
			_pCombineAnimationClip->uDuration = max(_mapClip[_CurClip]->uDuration, _mapClip[wcAttachClipName]->uDuration);
			_pCombineAnimationClip->uTickPerSecond = max(_mapClip[_CurClip]->uTickPerSecond, _mapClip[wcAttachClipName]->uTickPerSecond);
			// Set Max Frame.
			AkU32 uMaxKeyFrame = 0;
			for (AkU32 i = 0; i < _pCombineAnimationClip->uNumBoneAnimation; i++)
			{
				uMaxKeyFrame = max(uMaxKeyFrame, _pCombineAnimationClip->pBoneAnimationList[i].uNumKeyFrame);
			}
			_pCombineAnimationClip->uMaxKeyFrame = uMaxKeyFrame;

			{
				auto GetNameElement = [](const std::wstring& wcClipNAme) {
					size_t pos = wcClipNAme.find_last_of(L"_");
					std::wstring wcName = wcClipNAme.substr(pos + 1);
					return wcName;
					};

				std::wstring wcElementA = GetFileNmaeExcludeExt(GetNameElement(_CurClip));
				std::wstring wcElementB = GetFileNmaeExcludeExt(GetNameElement(wcAttachClipName));

				std::wstring wcSaveClipName = _CurCharacter + L"_" + wcElementA + L"_" + wcElementB + L".anim";

				_mapClip[wcSaveClipName] = _pCombineAnimationClip;

				ModifyAnimation(_CurCharacter, GetFileNmaeExcludeExt(wcSaveClipName));

				_mapClip.erase(wcSaveClipName);
			}
		}
	}
	ImGui::End();

	// Collider
	ImGui::Begin("Collider");
	ImGui::Checkbox("Box Collider Matching Min Max", &_bMatchingAABB);
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
		if (!_bRenderCharacter && !wcscmp(e->Name, _CurCharacter.c_str()))
		{
			continue;
		}

		e->Render();
	}

	_pGround->Render();

	for (auto& v : _vecColliders)
	{
		v->Render();
	}

	if (_pCurBoneAnimation && _bBindAnim && _bRenderBone)
	{
		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			Matrix mTransform = _mapAnim[_CurCharacter]->GetBoneTrnasformAtID(_pCurBoneAnimation[i].iID).Transpose() * _mapSkinnedModel[_CurCharacter]->GetWorldRow();

			_pCurBoneAnimation[i].mWorldRow = mTransform;

			_pCurBoneAnimation[i].Render();
		}
	}
}

void EditorModel::RenderShadow()
{
	for (auto& e : _vecModel)
	{
		if (!_bRenderCharacter && !wcscmp(e->Name, _CurCharacter.c_str()))
		{
			continue;
		}

		e->RenderShadow();
	}

	_pGround->RenderShadow();
}

void EditorModel::CleanUp()
{
	if (_pCombineAnimationClip)
	{
		delete _pCombineAnimationClip;
		_pCombineAnimationClip = nullptr;
	}

	for (auto& v : _vecColliders)
	{
		delete v;
		v = nullptr;
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


void EditorModel::Save(const std::wstring& wcFilePath)
{
	std::wstring wcPath = GetFilePath(wcFilePath);
	std::wstring wcPicked = L"";

	CreateFolders(ToString(wcPath));

	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFilePath.c_str(), L"wt");
	if (!fp) { __debugbreak(); }

	if (!_CurCharacter.empty() && _mapSkinnedModel[_CurCharacter]->IsPick())
	{
		wcPicked = _CurCharacter;

		fwprintf_s(fp, L"%d\n", 0);
	}
	else
	{
		for (auto& e : _mapBasicModel)
		{
			if (e.second->IsPick())
			{
				wcPicked = e.first;
			}
		}

		fwprintf_s(fp, L"%d\n", 1);
	}

	fwprintf_s(fp, L"%s\n", wcPicked.c_str());

	// Coliiders
	fwprintf_s(fp, L"%d\n", (AkI32)_mapColliders[wcPicked].size());
	for (auto& e : _mapColliders[wcPicked])
	{
		switch (e->GetType())
		{
		case COLLIDER_TYPE::BOX:
			fwprintf_s(fp, L"%d\n", 0);
			break;
		case COLLIDER_TYPE::SPHERE:
			fwprintf_s(fp, L"%d\n", 1);
			break;
		case COLLIDER_TYPE::CAPSULE:
			fwprintf_s(fp, L"%d\n", 2);
			break;
		}

		Vector3 vScale = e->GetTransform()->GetScale();
		Vector3 vRotation = e->GetTransform()->GetRotation();
		Vector3 vPosition = e->GetTransform()->GetPosition();

		fwprintf_s(fp, L"%lf %lf %lf\n", vScale.x, vScale.y, vScale.z);
		fwprintf_s(fp, L"%lf %lf %lf\n", vRotation.x, vRotation.y, vRotation.z);
		fwprintf_s(fp, L"%lf %lf %lf\n", vPosition.x, vPosition.y, vPosition.z);
	}

	if (fp) { fclose(fp); }
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

void EditorModel::CreateMeshFile(const std::wstring& wcName)
{
	Model* pPickID = nullptr;
	for (auto& e : _vecModel)
	{
		if (e->IsPick())
		{
			pPickID = e;
			break;
		}
	}

	AkU32 uMeshDataNum = 0;
	MeshData_t* pGeo = nullptr;

	switch (_iGeometryType)
	{
	case 0: // Sphere
	{
		pGeo = GeometryGenerator::MakeSphere(&uMeshDataNum, _fGeoSphereRadius, _uGeoSphereSlice, _uGeoSphereStack);

		SaveMesh(wcName, pGeo, uMeshDataNum, pPickID);
	}
	break;
	case 1: // Cube
	{
		pGeo = GeometryGenerator::MakeCube(&uMeshDataNum);

		SaveMesh(wcName, pGeo, uMeshDataNum, pPickID);
	}
	break;
	}

	GeometryGenerator::DestroyGeometry(pGeo, uMeshDataNum);
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
	using namespace std;

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

		_mapBasicModel[GetFileNmaeExcludeExt(wcFilename)] = pModel;
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

		_CurCharacter = GetFileNmaeExcludeExt(wcFilename);
	}

	wcscpy_s(pModel->Name, GetFileNmaeExcludeExt(wcFilename).c_str());
	Matrix mWorldRow = Matrix::CreateTranslation(Vector3(0.0f, 0.5f, 0.0f));
	pModel->UpdateWorldRow(&mWorldRow);

	_vecModel.push_back(pModel);

	Vector3 vMin = Vector3(-AK_MAX_F32);
	Vector3 vMax = Vector3(AK_MAX_F32);
	CalcColliderMinMax(pMeshData, uMeshDataNum, &vMin, &vMax);

	_mapAABB[GetFileNmaeExcludeExt(wcFilename)] = make_pair(vMin, vMax);

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
	Collider* pCollider = nullptr;

	switch (eType)
	{
	case COLLIDER_TYPE::BOX:
	{
		Model* pPickedModel = nullptr;
		for (auto& e : _vecModel)
		{
			if (e->IsPick())
			{
				pPickedModel = e;
			}
		}

		Vector3 vMin = Vector3(-0.5f);
		Vector3 vMax = Vector3(0.5f);
		if (_bMatchingAABB)
		{
			vMin = _mapAABB[pPickedModel->Name].first;
			vMax = _mapAABB[pPickedModel->Name].second;
		}
		
		pCollider = new BoxCollider(nullptr, &vMin, &vMax);

		_mapColliders[pPickedModel->Name].push_back(pCollider);
		_vecColliders.push_back(pCollider);
	}
	break;
	case COLLIDER_TYPE::SPHERE:
	{
		pCollider = new SphereCollider(nullptr);

		for (auto& e : _vecModel)
		{
			if (e->IsPick())
			{
				_mapColliders[e->Name].push_back(pCollider);
				_vecColliders.push_back(pCollider);
			}
		}
	}
	break;
	case COLLIDER_TYPE::CAPSULE:
	{
		pCollider = new CapsuleCollider(nullptr);

		for (auto& e : _vecModel)
		{
			if (e->IsPick())
			{
				_mapColliders[e->Name].push_back(pCollider);
				_vecColliders.push_back(pCollider);
			}
		}
	}
	break;
	}
}

void EditorModel::LoadTextures(const std::wstring& wcBasePath, const std::wstring& wcFilename)
{
	std::wstring wcFilePath = wcBasePath + wcFilename;

	wcFilePath = GetFileNmaeExcludeExt(wcFilePath);
	wcFilePath += L".dds";

	Model* pPickedModel = nullptr;
	for (auto& e : _vecModel)
	{
		if (e->IsPick())
		{
			pPickedModel = e;
		}
	}

	_mapTextures[pPickedModel].resize(8); // Texture Count 만큼

	switch (_iTextureType)
	{
	case 0: // Albedo
		pPickedModel->SetTextures(GRenderer->CreateTextureFromFile(wcFilePath.c_str(), AK_TRUE), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
		_mapTextures[pPickedModel][_iTextureType] = wcFilePath;
		break;
	case 1: // Emissvie
		pPickedModel->SetTextures(nullptr, GRenderer->CreateTextureFromFile(wcFilePath.c_str(), AK_TRUE), nullptr, nullptr, nullptr, nullptr, nullptr);
		_mapTextures[pPickedModel][_iTextureType] = wcFilePath;
		break;
	case 2: // Heighht
		pPickedModel->SetTextures(nullptr, nullptr, GRenderer->CreateTextureFromFile(wcFilePath.c_str(), AK_FALSE), nullptr, nullptr, nullptr, nullptr);
		_mapTextures[pPickedModel][_iTextureType] = wcFilePath;
		break;
	case 3: // Normal
		pPickedModel->SetTextures(nullptr, nullptr, nullptr, GRenderer->CreateTextureFromFile(wcFilePath.c_str(), AK_FALSE), nullptr, nullptr, nullptr);
		_mapTextures[pPickedModel][_iTextureType] = wcFilePath;
		break;
	case 4: // Metallic
		pPickedModel->SetTextures(nullptr, nullptr, nullptr, nullptr, GRenderer->CreateTextureFromFile(wcFilePath.c_str(), AK_FALSE), nullptr, nullptr);
		_mapTextures[pPickedModel][_iTextureType] = wcFilePath;
		break;
	case 5: // Roughneww
		pPickedModel->SetTextures(nullptr, nullptr, nullptr, nullptr, nullptr, GRenderer->CreateTextureFromFile(wcFilePath.c_str(), AK_FALSE), nullptr);
		_mapTextures[pPickedModel][_iTextureType] = wcFilePath;
		break;
	case 6: // AO
		pPickedModel->SetTextures(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, GRenderer->CreateTextureFromFile(wcFilePath.c_str(), AK_FALSE));
		_mapTextures[pPickedModel][_iTextureType] = wcFilePath;
		break;
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

	if (ImGuiFileDialog::Instance()->Display("DDSKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';


			SaveDDS(ToWString(FileName).c_str(), AK_FALSE);
		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("TextureKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

			LoadTextures(ToWString(FilePath), ToWString(GetFileName(FileName)));
		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("GeometryModelKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

			CreateMeshFile(ToWString(FileName));
		}

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("SaveActorDataKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath() + '\\';

			Save(ToWString(FileName));
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
			if (_pCurBoneAnimation[i].bPick)
			{
				_pCurBoneAnimation[i].mBoneTransform = _mapAnim[_CurCharacter]->GetBoneTrnasformAtID(i).Transpose() * _mapSkinnedModel[_CurCharacter]->GetWorldRow();
				_pCurBoneAnimation[i].RenderGUI();
			}
		}
	}

	for (auto& e : _mapColliders)
	{
		for (auto& v : e.second)
		{
			v->RenderGUI();
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
		_mapAnim[_CurCharacter]->PlayClip(wcClip.c_str(), ANIM_CLIP_STATE::LOOP, fSpeed, fBlendTime);
		_CurClip = wcClip;
		_fPrevAnimSpeed = fSpeed;
		_fPrevBlendTime = fBlendTime;
	}
}

void EditorModel::SaveMesh(const std::wstring& wcName, MeshData_t* pMeshData, AkU32 uMeshDataNum, void* pPickID)
{
	using namespace std;

	ofstream fout;
	fout.open(ToString(wcName).c_str());

	// write mesh data.
	fout << "========MeshData========" << endl;
	fout << "MeshCount: " << uMeshDataNum << endl;
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		fout << "VertexCount: " << pMeshData[i].uVerticeNum << "\t" << "IndexCount: " << pMeshData[i].uIndicesNum << endl;
	}
	fout << endl;

	// write material file name.
	fout << "========Material========" << endl;
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		fout << "Albedo: " << (ToString(_mapTextures[pPickID][0]).empty() ? "Empty" : GetFileNmaeExcludeExt(ToString(_mapTextures[pPickID][0])) + ".dds") << endl;
		fout << "Emissive: " << (ToString(_mapTextures[pPickID][1]).empty() ? "Empty" : GetFileNmaeExcludeExt(ToString(_mapTextures[pPickID][1])) + ".dds") << endl;
		fout << "Height: " << (ToString(_mapTextures[pPickID][2]).empty() ? "Empty" : GetFileNmaeExcludeExt(ToString(_mapTextures[pPickID][2])) + ".dds") << endl;
		fout << "Normal: " << (ToString(_mapTextures[pPickID][3]).empty() ? "Empty" : GetFileNmaeExcludeExt(ToString(_mapTextures[pPickID][3])) + ".dds") << endl;
		fout << "Metallic: " << (ToString(_mapTextures[pPickID][4]).empty() ? "Empty" : GetFileNmaeExcludeExt(ToString(_mapTextures[pPickID][4])) + ".dds") << endl;
		fout << "Roughness: " << (ToString(_mapTextures[pPickID][5]).empty() ? "Empty" : GetFileNmaeExcludeExt(ToString(_mapTextures[pPickID][5])) + ".dds") << endl;
		fout << "Ao: " << (ToString(_mapTextures[pPickID][6]).empty() ? "Empty" : GetFileNmaeExcludeExt(ToString(_mapTextures[pPickID][6])) + ".dds") << endl;
		fout << "Opacity: " << (ToString(_mapTextures[pPickID][7]).empty() ? "Empty" : GetFileNmaeExcludeExt(ToString(_mapTextures[pPickID][7])) + ".dds") << endl;
		fout << endl;
	}

	// write vertices and indices.
	fout << "========Vertices========" << endl;
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < pMeshData[i].uVerticeNum; j++)
		{
			fout << "Position: " << pMeshData[i].pVertices[j].vPosition.x << " " << pMeshData[i].pVertices[j].vPosition.y << " " << pMeshData[i].pVertices[j].vPosition.z << endl;
			fout << "Normal: " << pMeshData[i].pVertices[j].vNormalModel.x << " " << pMeshData[i].pVertices[j].vNormalModel.y << " " << pMeshData[i].pVertices[j].vNormalModel.z << endl;
			fout << "Texcoord: " << pMeshData[i].pVertices[j].vTexCoord.x << " " << pMeshData[i].pVertices[j].vTexCoord.y << endl;
			fout << "Tangent: " << pMeshData[i].pVertices[j].vTangentModel.x << " " << pMeshData[i].pVertices[j].vTangentModel.y << " " << pMeshData[i].pVertices[j].vTangentModel.z << endl;
			fout << endl;
		}
	}
	fout << "========Indices========" << endl;
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < pMeshData[i].uIndicesNum; j += 3)
		{
			fout << pMeshData[i].pIndices[j] << " " << pMeshData[i].pIndices[j + 1] << " " << pMeshData[i].pIndices[j + 2] << endl;
		}
	}
	fout << endl;

	// write bone offset matrix
	fout << "========BoneOffsets========" << endl;
	fout << "BoneCount: " << 0 << endl;

	// write bone hierarchy
	fout << "========BoneHierarchy========" << endl;
	fout << endl;

	// write bone name
	fout << "========BoneName========" << endl;
	fout << endl;

	if (fout.is_open())
	{
		fout.close();
	}
}

void EditorModel::DeleteProcess()
{
	AkU32 uIndex = 0;
	for (auto& e : _vecModel)
	{
		if (e->IsPick())
		{
			if (KEY_DOWN(KEY_INPUT_DELETE))
			{
				if (_mapBasicModel[e->Name])
				{
					_mapBasicModel.erase(e->Name);
				}
				if (_mapSkinnedModel[e->Name])
				{
					_mapSkinnedModel.erase(e->Name);
				}

				_vecModel.erase(_vecModel.begin() + uIndex);

				if (e)
				{
					delete e;
					e = nullptr;
				}
			}

			uIndex++;
		}
	}

	uIndex = 0;
	for (auto& e : _vecColliders)
	{
		if (e->IsPick())
		{
			if (KEY_DOWN(KEY_INPUT_DELETE))
			{
				AkU32 uIndex2 = 0;
				for (auto& v0 : _mapColliders)
				{
					for (auto& v1 : v0.second)
					{
						if (v1 == e)
						{
							v0.second.erase(v0.second.begin() + uIndex2);
						}
						uIndex2++;
					}
				}

				_vecColliders.erase(_vecColliders.begin() + uIndex);

				if (e)
				{
					delete e;
					e = nullptr;
				}
			}
		}

		uIndex++;
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

