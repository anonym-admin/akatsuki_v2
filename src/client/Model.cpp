#include "pch.h"
#include "Model.h"
#include "AssetManager.h"
#include "ModelExporter.h"

/*
=================
ImGizmo Variable
=================
*/

static bool useSnap(false);
static float snap[3] = { 1.f, 1.f, 1.f };
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

/*
========
Model
========
*/

Model::Model(const Model& Other)
{
	_uRefCount++;
}

Model::Model(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	if (!Initialize(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive))
	{
		__debugbreak();
	}
}

Model::Model(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	if (!Initialize(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive))
	{
		__debugbreak();
	}
}

Model::~Model()
{
	CleanUp();
}

AkBool Model::Initialize(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	CreateMeshObject(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum);
	CreateMaterial(pAlbedo, fMetallic, fRoughness, pEmissive);
	return AK_TRUE;
}

AkBool Model::Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	CreateMeshObject(pMeshData, uMeshDataNum);
	CreateMaterial(pAlbedo, fMetallic, fRoughness, pEmissive);
	return AK_TRUE;
}

void Model::Render()
{
	GRenderer->RenderBasicMeshObject(_pMeshObj, &_mWorldRow);

	GRenderer->RenderReflectionOfBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::RenderNormals()
{
	GRenderer->RenderNormalOfBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::RenderShadowMaps()
{
	if(_bDrawShadow)
		GRenderer->RenderShadowOfBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::RenderDepthMap()
{
	GRenderer->RenderDepthMapOfBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::RenderGUI()
{
	std::wstring ModelName = Name;
	char Title[_MAX_PATH] = {};
	strcpy_s(Title, ToString(ModelName + L" gizmo").c_str());

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
	ImGuizmo::DecomposeMatrixToComponents((float*)&_mWorldRow._11, matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation);
	ImGui::InputFloat3("Rt", matrixRotation);
	ImGui::InputFloat3("Sc", matrixScale);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&_mWorldRow._11);

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

	ImGuizmo::Manipulate((float*)&mView, (float*)&mProj, mCurrentGizmoOperation, mCurrentGizmoMode, (float*)&_mWorldRow._11, NULL, useSnap ? &snap[0] : NULL);

	if (ImGui::SliderFloat("IBL Strength", &_fIBLStrength, 0.0f, 1.0f))
		SetIBLStrength(_fIBLStrength);

	ImGui::End();
}

void Model::UpdateWorldRow(Matrix* pWorldRow)
{
	_mWorldRow = *pWorldRow;
}

void Model::SetWireFrame(AkBool bDrawWire)
{
	if (bDrawWire)
		_pMeshObj->EnableWireFrame();
	else
		_pMeshObj->DisableWireFrame();
}

void Model::SetTextures(void* pAlbedo, void* pEmissve, void* pHeight, void* pNormal, void* pMetallic, void* pRoughness, void* pAO)
{
	_pMeshObj->SetTextures(pAlbedo, pEmissve, pHeight, pNormal, pMetallic, pRoughness, pAO);
}

void Model::SetIBLStrength(AkF32 fIBLStrength)
{
	_fIBLStrength = fIBLStrength;

	_pMeshObj->SetIBLStrength(_fIBLStrength);
}

void Model::operator=(const Model& Other)
{
	_uRefCount++;
}

AkU32 Model::AddRef() // 증가되기전의 RefCount를 반환한다.
{
	AkU32 uRefCount = _uRefCount++;
	return uRefCount;
}

AkU32 Model::Release()
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
	return uRefCount;
}

void Model::CleanUp()
{
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}
}

void Model::CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	_pMeshObj = GRenderer->CreateBasicMeshObject();
	_pMeshObj->CreateMeshBuffers(pMeshData, uMeshDataNum);
}

void Model::CreateMaterial(const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	_pMeshObj->UpdateMaterialBuffers(pAlbedo, fMetallic, fRoughness, pEmissive);
}

