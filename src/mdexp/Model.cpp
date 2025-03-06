#include "Model.h"
#include "SkinnedModel.h"
#include <imgui.h>
#include <ImGuizmo.h>

extern IRenderer* g_pRenderer;
extern bool g_bEnableAnim;
extern bool g_bAttachWeaponLeftHand;
extern bool g_bAttachWeaponRightHand;
extern int g_iMouseX;
extern int g_iMouseY;
extern float g_fNdcX;
extern float g_fNdcY;
extern bool g_bChecked;

extern Matrix g_mViewMat;
extern Matrix g_mProjMat;

static bool useSnap(false);
static float snap[3] = { 1.f, 1.f, 1.f };

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

Model::Model()
{
}

Model::~Model()
{
	CleanUp();
}

bool Model::Init(MeshData_t* pMeshData, int iMeshCount)
{
	_pMeshObj = g_pRenderer->CreateBasicMeshObject();

	Vector3 vAlbedo = Vector3(1.0f);
	AkF32 fMetallic = 0.0f;
	AkF32 fRoughness = 1.0f;
	Vector3 vEmission = Vector3(0.0f);

	_pMeshObj->CreateMeshBuffers(pMeshData, iMeshCount);
	_pMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmission);

	BndBox.Center = Vector3(0.0f);
	BndBox.Extents = Vector3(0.5f, 0.5f, 0.5f);

	return true;
}

void Model::Update(const float fDT)
{
	_mBoneTransform = Matrix();

	if (IsWeapon)
	{
		if (pOwner && g_bEnableAnim)
		{
			if (LeftHand)
			{
				_mBoneTransform = _mLeftHandBoneTransform;
			}
			if (RightHand)
			{
				_mBoneTransform = _mRightHandBoneTransform;
			}
		}
	}

	_mGizmoWorldRow = _mWorldRow * _mBoneTransform;

	UpdateGui();
	UpdateGizmo();

	_mFinalWorldRow = _mWorldRow * _mBoneTransform;

	// Update Bounding Box Position.
	BndBox.Center = _mWorldRow.Translation();
}

void Model::Render()
{
	g_pRenderer->RenderBasicMeshObject(_pMeshObj, &_mFinalWorldRow);
}

void Model::RenderShadow()
{
	g_pRenderer->RenderShadowOfBasicMeshObject(_pMeshObj, &_mFinalWorldRow);
}

void Model::SetLeftHandBoneTransform(Matrix mBoneTransform)
{
	_mLeftHandBoneTransform = mBoneTransform;
	LeftHand = true;
}

void Model::SetRightHandBoneTransform(Matrix mBoneTransform)
{
	_mRightHandBoneTransform = mBoneTransform;
	RightHand = true;
}

void Model::CleanUp()
{
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}
}

void Model::UpdateGui()
{
	if (!Picked)
	{
		return;
	}

	ImGuiIO& io = ImGui::GetIO();

	string strBegin = Name + " Model info";
	ImGui::Begin(strBegin.c_str());
	ImGui::Checkbox("Wire", &IsWire);
	if (IsWeapon)
	{
		if (ImGui::Checkbox("Attach Character int Left Hand", &g_bAttachWeaponLeftHand))
		{
			if (!g_bAttachWeaponLeftHand)
			{
				g_bChecked = false;
				SkinnedModel* Owner = (SkinnedModel*)pOwner;
				Owner->UseLeftHand = false;
			}
		}
		if (ImGui::Checkbox("Attach Character int Right Hand", &g_bAttachWeaponRightHand))
		{
			if (!g_bAttachWeaponRightHand)
			{
				g_bChecked = false;
				SkinnedModel* Owner = (SkinnedModel*)pOwner;
				Owner->UseRightHand = false;
			}
		}
	}

	if (IsWire)
	{
		_pMeshObj->EnableWireFrame();
	}
	else
	{
		_pMeshObj->DisableWireFrame();
	}

	ImGui::End();
}

void Model::UpdateGizmo()
{
	if (!Picked)
	{
		return;
	}

	// Gizmo.
	string strBegin = Name + " Gizmo";
	ImGuizmo::BeginFrame();
	ImGui::Begin(strBegin.c_str());
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

	if (ImGui::IsKeyPressed(ImGuiKey_T))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_E))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) // r Key
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
	ImGuizmo::DecomposeMatrixToComponents((float*)&_mGizmoWorldRow._11, matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation);
	ImGui::InputFloat3("Rt", matrixRotation);
	ImGui::InputFloat3("Sc", matrixScale);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&_mGizmoWorldRow._11);

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_S))
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

	g_mViewMat = g_mViewMat.Transpose();
	g_mProjMat = g_mProjMat.Transpose();

	ImGuizmo::Manipulate((float*)&g_mViewMat, (float*)&g_mProjMat, mCurrentGizmoOperation, mCurrentGizmoMode, (float*)&_mGizmoWorldRow._11, NULL, useSnap ? &snap[0] : NULL);

	_mWorldRow = _mGizmoWorldRow * _mBoneTransform.Invert();

	ImGui::End();
}

