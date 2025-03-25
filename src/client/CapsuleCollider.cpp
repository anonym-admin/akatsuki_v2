#include "pch.h"
#include "CapsuleCollider.h"
#include "Transform.h"
#include "BoxCollider.h"

static bool useSnap(false);
static float snap[3] = { 1.f, 1.f, 1.f };
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

/*
====================
Capsule Collider
====================
*/

CapsuleCollider::CapsuleCollider(Actor* pOwner, AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
	: Collider(pOwner)
{
	if (!Initialize(fRadius, fHeight, uStack, uSlice, pColor))
	{
		__debugbreak();
	}
}

CapsuleCollider::~CapsuleCollider()
{
}

AkBool CapsuleCollider::Initialize(AkF32 fRadius, AkF32 fHeight, AkU32 uStack, AkU32 uSlice, const Vector3* pColor)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	if (pColor)
		vColor = *pColor;

	_eType = COLLIDER_TYPE::CAPSULE;

	LineData_t* pLineSphere = GeometryGenerator::MakeCapsule(fRadius, fHeight, uSlice, uStack, &vColor);
	_pLineObj = GRenderer->CreateLineObject();
	_pLineObj->CreateLineBuffers(pLineSphere);
	GeometryGenerator::DestroyGeometry(pLineSphere);

	_fRadius = fRadius;
	_fHeight = fHeight;

	return AK_TRUE;
}

AkBool CapsuleCollider::RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos, AkF32* pOutDist)
{
	return AkBool();
}

AkBool CapsuleCollider::BoxIntersect(BoxCollider* pCollider)
{
	Vector3 vUp = _pTransform->Up();
	Vector3 vO = _pTransform->GetGlobalPosition() - vUp * Height() * 0.5f;

	Vector3 vA = pCollider->GetTransform()->GetGlobalPosition() - vO;

	AkF32 t = vA.Dot(vUp);
	t = max(0.0f, t);
	t = min(Height(), t);

	Vector3 vP = vO + vUp * t;

	return pCollider->SphereIntersect(&vP, Radius());
}

AkBool CapsuleCollider::SphereIntersect(SphereCollider* pCollider)
{
	return AkBool();
}

AkBool CapsuleCollider::CapsuleIntersect(CapsuleCollider* pCapsule)
{
	return AkBool();
}

void CapsuleCollider::OnCollisionEnter(Collider* pCollider)
{
	Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollisionEnter(pCollider);
}

void CapsuleCollider::OnCollision(Collider* pCollider)
{
	Vector3 vColor = Vector3(1.0f, 0.0f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollision(pCollider);
}

void CapsuleCollider::OnCollisionExit(Collider* pCollider)
{
	Vector3 vColor = Vector3(0.0f, 0.5f, 0.0f);
	SetColor(&vColor);

	_pOwner->OnCollisionExit(pCollider);
}

void CapsuleCollider::RenderGUI()
{
	std::wstring ModelName = L"Capsule Collider_" + std::to_wstring(GetID());
	char Title[_MAX_PATH] = {};
	strcpy_s(Title, ToString(ModelName + L" gizmo").c_str());

	Matrix& mWorldRow = _pTransform->GetWorldTransform();

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

	Vector3 vScale = Vector3(1.0f);
	Quaternion qRotation = Quaternion();
	Vector3 vPosition = Vector3(0.0f);

	mWorldRow.Decompose(vScale, qRotation, vPosition);

	_pTransform->SetScale(&vScale);

	ImGui::End();
}

AkF32 CapsuleCollider::Radius()
{
	Vector3 vScale = _pTransform->GetScale();
	return _fRadius * max(vScale.x, max(vScale.y, vScale.z));
}

AkF32 CapsuleCollider::Height()
{
	Vector3 vScale = _pTransform->GetScale();
	return _fHeight * vScale.y;
}