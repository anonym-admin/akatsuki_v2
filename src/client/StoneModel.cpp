#include "pch.h"
#include "StoneModel.h"

StoneModel::StoneModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
    : Model(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive)
{
    if (!Initialize())
    {
        __debugbreak();
    }
}

StoneModel::StoneModel(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
    : Model(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive)
{
    if (!Initialize())
    {
        __debugbreak();
    }
}

StoneModel::~StoneModel()
{

}

AkBool StoneModel::Initialize()
{
    wcscpy_s(Name, L"Stone");

    return AK_TRUE;
}

void StoneModel::Render()
{
    if (_fHeightScale != 0.0f)
        _pMeshObj->SetHeightScale(_fHeightScale);

    Model::Render();
}

void StoneModel::RenderGUI()
{
    std::wstring ModelName = Name + std::wstring(L"_" + std::to_wstring(_uRefCount));
    char Title[_MAX_PATH] = {};
    strcpy_s(Title, ToString(ModelName + L" edit").c_str());

    ImGui::Begin(Title);
    ImGui::SliderFloat("Height Scale", &_fHeightScale, 0.0f, 1.0f);
    ImGui::End();
}
