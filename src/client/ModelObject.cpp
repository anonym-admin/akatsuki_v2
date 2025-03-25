#include "pch.h"
#include "ModelObject.h"
#include "Swat.h"

static bool useSnap(false);
static float snap[3] = { 1.f, 1.f, 1.f };
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

/*
=================
Model Object
=================
*/

ModelObject::ModelObject(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsAnim)
{
	if (!Initialize(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive, bIsAnim))
	{
		__debugbreak();
	}
}

ModelObject::ModelObject(const wchar_t* wcFilename, AkBool bIsAnim)
{
	if (!Initialize(wcFilename, bIsAnim))
	{
		__debugbreak();
	}
}

ModelObject::ModelObject(const wchar_t* wcScript)
{
	if (!Initialize(wcScript))
	{
		__debugbreak();
	}
}

ModelObject::~ModelObject()
{
	CleanUp();
}

AkBool ModelObject::Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsAnim)
{
	// Create Model
	_pModel = CreateModel(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive, bIsAnim);

	// Create Transform
	_pTransform = CreateTransform();

	// Create Collider.
	Vector3 vMin = Vector3(-0.5f);
	Vector3 vMax = Vector3(0.5f);
	CalcColliderMinMax(pMeshData, uMeshDataNum, &vMin, &vMax);
	_pCollider = CreateBoxCollider(&vMin, &vMax);

	return AK_TRUE;
}

AkBool ModelObject::Initialize(const wchar_t* wcFilename, AkBool bIsAnim)
{
	AkU32 uMeshDataNum = 0;
	MeshData_t* pMeshData = GeometryGenerator::ReadFromFile(&uMeshDataNum, MODEL_FILE_PATH, wcFilename, bIsAnim);
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissive = Vector3(0.0f);
	_pModel = CreateModel(pMeshData, uMeshDataNum, &vAlbedo, 1.0f, 0.0f, &vEmissive, bIsAnim);

	// Create Transform
	_pTransform = CreateTransform();

	// Create Collider.
	Vector3 vMin = Vector3(-0.5f);
	Vector3 vMax = Vector3(0.5f);
	CalcColliderMinMax(pMeshData, uMeshDataNum, &vMin, &vMax);
	_pCollider = CreateBoxCollider(&vMin, &vMax);

	return AK_TRUE;
}

AkBool ModelObject::Initialize(const wchar_t* wcScript)
{
	FILE* fp = nullptr;
	_wfopen_s(&fp, wcScript, L"rt");
	if (!fp) { __debugbreak(); }

	AkI32 iIsSkinned = 0;
	fwscanf_s(fp, L"%d\n", &iIsSkinned);
	if (!iIsSkinned)
	{
		wchar_t wcFilePath[_MAX_PATH] = {};

		fwscanf_s(fp, L"%s\n", wcFilePath, _MAX_PATH);
		wcscpy_s(_wcModelName, wcFilePath);
		wcscat_s(wcFilePath, L".mesh");

		AkU32 uMeshDataNum = 0;
		MeshData_t* pMeshData = GeometryGenerator::ReadFromFile(&uMeshDataNum, L"../../assets/model_new/mesh/", wcFilePath, AK_TRUE, &_mDefaultMatrix, &_pBoneOffsetMatrixList, &_pBoneHierarchyList, &_uBoneNum, &_ppBoneName);
		Vector3 vAlbedo = Vector3(1.0f);
		Vector3 vEmissive = Vector3(0.0f);
		_pModel = CreateModel(pMeshData, uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_TRUE);

		GeometryGenerator::DestroyGeometry(pMeshData, uMeshDataNum);
	}
	else
	{
		wchar_t wcFilePath[_MAX_PATH] = {};

		fwscanf_s(fp, L"%s\n", wcFilePath, _MAX_PATH);
		wcscpy_s(_wcModelName, wcFilePath);
		wcscat_s(wcFilePath, L".mesh");

		AkU32 uMeshDataNum = 0;
		MeshData_t* pMeshData = GeometryGenerator::ReadFromFile(&uMeshDataNum, L"../../assets/model_new/mesh/", wcFilePath, AK_FALSE);
		Vector3 vAlbedo = Vector3(1.0f);
		Vector3 vEmissive = Vector3(0.0f);
		_pModel = CreateModel(pMeshData, uMeshDataNum, &vAlbedo, 0.0f, 1.0f, &vEmissive, AK_FALSE);

		GeometryGenerator::DestroyGeometry(pMeshData, uMeshDataNum);
	}


	// Collider °¹¼ö ÆÄ½Ì ÇÊ¿ä!!
	AkI32 iColliderNum = 0;
	fwscanf_s(fp, L"%d\n", &iColliderNum);
	_uEventColliderNum = (AkU32)iColliderNum - 1;

	for (AkI32 i = 0; i < iColliderNum; i++)
	{
		Collider* pCollider = nullptr;
		AkI32 iColliderType = 0;
		fwscanf_s(fp, L"%d\n", &iColliderType);

		switch (iColliderType)
		{
		case (AkI32)COLLIDER_TYPE::BOX:
			pCollider = CreateBoxCollider();
			break;
		case (AkI32)COLLIDER_TYPE::SPHERE:
			pCollider = CreateSphereCollider();
			break;
		case (AkI32)COLLIDER_TYPE::CAPSULE:
			pCollider = CreateCapsuleCollider();
			break;
		}

		Vector3 vScale = Vector3(1.0f);
		Vector3 vRotation = Vector3(0.0f);
		Vector3 vPosition = Vector3(0.0f);

		fwscanf_s(fp, L"%f %f %f\n", &vScale.x, &vScale.y, &vScale.z);
		fwscanf_s(fp, L"%f %f %f\n", &vRotation.x, &vRotation.y, &vRotation.z);
		fwscanf_s(fp, L"%f %f %f\n", &vPosition.x, &vPosition.y, &vPosition.z);

		pCollider->GetTransform()->SetScale(&vScale);
		pCollider->GetTransform()->SetRotation(&vRotation);
		pCollider->GetTransform()->SetPosition(&vPosition);

		if (0 == i)
		{
			_pCollider = pCollider;
		}
		else
		{
			_pEventCollider[i - 1] = pCollider;
		}
	}

	if (fp) { fclose(fp); }

	// Create Transform
	_pTransform = CreateTransform();

	return AK_TRUE;
}

void ModelObject::Update()
{
}

void ModelObject::FinalUpdate()
{
	if(!_bEditMode)
	{
		_pTransform->Update();
	}
	
	_pCollider->Update();
	for (AkU32 i = 0; i < _uEventColliderNum; i++)
	{
		_pEventCollider[i]->Update();
	}

	_pModel->UpdateWorldRow(_pTransform->GetWorldTransformAddr());
}

void ModelObject::Render()
{
	_pModel->Render();

	_pCollider->Render();
	for (AkU32 i = 0; i < _uEventColliderNum; i++)
	{
		_pEventCollider[i]->Render();
	}
}

void ModelObject::RenderShadow()
{
	_pModel->RenderShadow();
}

void ModelObject::RenderGUI()
{
	if (!_bEditMode)
	{
		return;
	}

	std::wstring ModelName = _wcModelName;
	char Title[_MAX_PATH] = {};
	strcpy_s(Title, ToString(ModelName + L" gizmo").c_str());

	ImGuizmo::BeginFrame();
	ImGui::Begin(Title);
	ImGui::Checkbox("Use Gizmo", &_bUseGizmo);

	Matrix& mWorldRow = _pTransform->GetWorldTransform();

	if (!_bUseGizmo)
	{
		ImGui::End();
		return;
	}

	if (ImGuizmo::IsUsing())
	{
		ImGui::Text("Using gizmo");
	}
	else
	{
		ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Z))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_X))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_C)) // r Key
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents((float*)&mWorldRow._11, matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation);
	ImGui::InputFloat3("Rt", matrixRotation);
	ImGui::InputFloat3("Sc", matrixScale);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&mWorldRow._11);

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_V))
		useSnap = !useSnap;
	ImGui::Checkbox(" ", &useSnap);
	ImGui::SameLine();
	switch (mCurrentGizmoOperation)
	{
	case ImGuizmo::TRANSLATE:
		ImGui::InputFloat3("Snap", &snap[0]);
		break;
	case ImGuizmo::ROTATE:
		ImGui::InputFloat("Angle Snap", &snap[0]);
		break;
	case ImGuizmo::SCALE:
		ImGui::InputFloat("Scale Snap", &snap[0]);
		break;
	}

	float windowWidth = (float)ImGui::GetWindowWidth();
	float windowHeight = (float)ImGui::GetWindowHeight();

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	Matrix mView = Matrix();
	Matrix mProj = Matrix();
	GRenderer->GetViewPorjMatrix(&mView, &mProj);
	mView = mView.Transpose();
	mProj = mProj.Transpose();

	ImGuizmo::Manipulate((float*)&mView, (float*)&mProj, mCurrentGizmoOperation, mCurrentGizmoMode, (float*)&mWorldRow._11, NULL, useSnap ? &snap[0] : NULL);

	ImGui::End();
}

void ModelObject::OnCollisionEnter(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"Swat"))
	{
		((Swat*)pOtherOwner)->ActionReaction(_pCollider);
	}
}

void ModelObject::OnCollision(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	if (!wcscmp(pOtherOwner->Name, L"Swat"))
	{
		((Swat*)pOtherOwner)->ActionReaction(_pCollider);
	}
}

void ModelObject::OnCollisionExit(Collider* pOther)
{
}

ModelObject* ModelObject::Clone()
{
	Spawn::Clone();
	return new ModelObject();
}

void ModelObject::CleanUp()
{
	for (AkU32 i = 0; i < _uEventColliderNum; i++)
	{
		if (_pEventCollider[i])
		{
			delete _pEventCollider[i];
			_pEventCollider[i] = nullptr;
		}
	}

	if (!_pAnimation)
	{
		if (_pBoneOffsetMatrixList)
		{
			delete[] _pBoneOffsetMatrixList;
			_pBoneOffsetMatrixList = nullptr;
		}
		if (_pBoneHierarchyList)
		{
			delete[] _pBoneHierarchyList;
			_pBoneHierarchyList = nullptr;
		}

		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			free(_ppBoneName[i]);
			_ppBoneName[i] = nullptr;
		}

		free(_ppBoneName);
		_ppBoneName = nullptr;
	}
}
