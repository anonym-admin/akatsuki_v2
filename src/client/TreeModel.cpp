#include "pch.h"
#include "TreeModel.h"

TreeModel::TreeModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
	: Model(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive)
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

TreeModel::TreeModel(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
	: Model(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive)
{
}

TreeModel::~TreeModel()
{
}

AkBool TreeModel::Initialize()
{
	wcscpy_s(Name, L"Tree");

	return AK_TRUE;
}

void TreeModel::Render()
{
	if(_fWindTrunk != 0.0f)
		_pMeshObj->SetWindTrunk(_fWindTrunk);


	Model::Render();
}

void TreeModel::RenderGUI()
{
	std::wstring ModelName = Name + std::wstring(L"_" + std::to_wstring(_uRefCount));
	char Title[_MAX_PATH] = {};
	strcpy_s(Title, ToString(ModelName + L" edit").c_str());

	ImGui::Begin(Title);
	ImGui::SliderFloat("Wind Trunk", &_fWindTrunk, 0.0f, 1.0f);
	ImGui::SliderFloat("Wind Leaves", &_fWindLeaves, 0.0f, 1.0f);
	ImGui::End();
}
