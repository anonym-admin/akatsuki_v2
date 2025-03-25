#include "pch.h"
#include "BoxCollider.h"
#include "Transform.h"
#include "CapsuleCollider.h"

static bool useSnap(false);
static float snap[3] = { 1.f, 1.f, 1.f };
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

/*
====================
Box Collider
====================
*/

BoxCollider::BoxCollider(Actor* pOwner, const Vector3* pMin, const Vector3* pMax, const Vector3* pColor)
	: Collider(pOwner)
{
	if (!Initialize(pMin, pMax, pColor))
	{
		__debugbreak();
	}
}

BoxCollider::~BoxCollider()
{
}

AkBool BoxCollider::Initialize(const Vector3* pMin, const Vector3* pMax, const Vector3* pColor)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	if (pColor)
		vColor = *pColor;

	// Set type.
	_eType = COLLIDER_TYPE::BOX;

	// Create the render obj.
	LineData_t* pLineBox = GeometryGenerator::MakeCube(pMin, pMax, &vColor);
	_pLineObj = GRenderer->CreateLineObject();
	_pLineObj->CreateLineBuffers(pLineBox);
	GeometryGenerator::DestroyGeometry(pLineBox);

	_vMin = *pMin;
	_vMax = *pMax;

	return AK_TRUE;
}

AkBool BoxCollider::RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos, AkF32* pOutDist)
{
	return AkBool();
}

AkBool BoxCollider::BoxIntersect(BoxCollider* pCollider)
{
	return AkBool();
}

AkBool BoxCollider::SphereIntersect(SphereCollider* pCollider)
{
	return AkBool();
}

AkBool BoxCollider::CapsuleIntersect(CapsuleCollider* pCollider)
{
	return pCollider->BoxIntersect(this);
}

AkBool BoxCollider::SphereIntersect(const Vector3* pCenter, AkF32 fRadius)
{
	Vector3 vPos = _pTransform->GetGlobalPosition();
	Quaternion vQuat = _pTransform->GetGlobalRotation();

	Matrix mTranslation = Matrix::CreateTranslation(vPos);
	Matrix mRotation = Matrix::CreateFromQuaternion(vQuat);
	
	Matrix mInvWorld = (mRotation * mTranslation).Invert();

	Vector3 vSpherePos = Vector3::Transform(*pCenter, mInvWorld); // To Model Coord.

	Vector3 vMin = _vMin * _pTransform->GetScale();
	Vector3 vMax = _vMax * _pTransform->GetScale();

	Vector3 vPoint = Vector3(0.0f);
	vPoint.x = max(vMin.x, min(vSpherePos.x, vMax.x));
	vPoint.y = max(vMin.y, min(vSpherePos.y, vMax.y));
	vPoint.z = max(vMin.z, min(vSpherePos.z, vMax.z));

	vPoint -= vSpherePos;

	return vPoint.Length() <= fRadius;
}

void BoxCollider::OnCollisionEnter(Collider* pCollider)
{
	Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollisionEnter(pCollider);
}

void BoxCollider::OnCollision(Collider* pCollider)
{
	Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollision(pCollider);
}

void BoxCollider::OnCollisionExit(Collider* pCollider)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollisionExit(pCollider);
}

void BoxCollider::RenderGUI()
{
	// 스케일과 회전만 조정 가능
	// 회전은 현재 미구현
	// 이동의 경우 Parent 에서 처리됨.
	std::wstring ModelName = L"Box Collider_" + std::to_wstring(GetID());
	char Title[_MAX_PATH] = {};
	strcpy_s(Title, ToString(ModelName + L" gizmo").c_str());

	Vector3 vScale = _pTransform->GetScale();
	Quaternion qRotation = Quaternion();
	Vector3 vPosition = _pTransform->GetPosition();

	Matrix mWorldRow = Matrix::CreateScale(vScale) * Matrix::CreateTranslation(vPosition);

	ImGuizmo::BeginFrame();
	ImGui::Begin(Title);
	ImGui::Checkbox("Use Gizmo", &_bUseGizmo);

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

	mWorldRow.Decompose(vScale, qRotation, vPosition);

	_pTransform->SetScale(&vScale);
	_pTransform->SetPosition(&vPosition);

	ImGui::End();
}

Vector3 BoxCollider::GetMinWorld()
{
	Vector3 vMinWorld = Vector3(0.0f);
	vMinWorld = Vector3::Transform(_vMin, _pTransform->GetWorldTransform());
	return vMinWorld;
}

Vector3 BoxCollider::GetMaxWorld()
{
	Vector3 vMaxWorld = Vector3(0.0f);
	vMaxWorld = Vector3::Transform(_vMax, _pTransform->GetWorldTransform());
	return vMaxWorld;
}

AkF32 BoxCollider::Radius()
{
	Vector3 vRadius = (GetMaxWorld() - GetMinWorld()) * 0.5f;
	return vRadius.Length();
}



