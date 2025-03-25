#include "pch.h"
#include "ModelObject.h"

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
	if (!Actor::Initialize(wcScript))
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
	MeshData_t* pMeshData = GeometryGenerator::ReadFromFile(&uMeshDataNum, MESH_FILE_PATH, wcFilename, bIsAnim);
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

	std::wstring ModelName = Name;
	char Title[_MAX_PATH] = {};
	strcpy_s(Title, ToString(ModelName + L" gizmo").c_str());

	ImGuizmo::BeginFrame();
	ImGui::Begin(Title);
	ImGui::Checkbox("Use Gizmo", &_bUseGizmo);

	Matrix mWorldRow = _pTransform->GetWorldTransform();

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

	ImGuizmo::DecomposeMatrixToComponents((float*)&mWorldRow._11, matrixTranslation, matrixRotation, matrixScale);

	_pTransform->SetScale((Vector3*)matrixScale);
	_pTransform->SetRotation(DirectX::XMConvertToRadians(matrixRotation[1]), 0.0f, 0.0f);
	_pTransform->SetPosition((Vector3*)matrixTranslation);

	_pTransform->Update();

	ImGui::End();
}

void ModelObject::OnCollisionEnter(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	//if (!wcscmp(pOtherOwner->Name, L"Swat"))
	//{
	//	((Soldier*)pOtherOwner)->ActionReaction(_pCollider);
	//}
}

void ModelObject::OnCollision(Collider* pOther)
{
	Actor* pOtherOwner = pOther->GetOwner();
	//if (!wcscmp(pOtherOwner->Name, L"Swat"))
	//{
	//	((Soldier*)pOtherOwner)->ActionReaction(_pCollider);
	//}
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
}
