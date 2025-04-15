#include "pch.h"
#include "Cloud.h"

/*
=======
Cloud
=======
*/

Cloud::Cloud()
{
    if (!Initialize())
    {
        __debugbreak();
    }
}

Cloud::~Cloud()
{
    CleanUp();
}

AkBool Cloud::Initialize()
{
    // Create Cube
    Vector3 vMin = Vector3(-1.0f);
    Vector3 vMax = Vector3(1.0f);
    MeshData_t* pCube = GeometryGenerator::MakeCube(&vMin, &vMax);
    _pCloudObj = GRenderer->CreateCloudObject();
    _pCloudObj->CreateMeshBuffers(pCube, 1);
    GeometryGenerator::DestroyGeometry(pCube, 1);

    // Create Transform
    _pTransform = CreateTransform();

    return AK_TRUE;
}

void Cloud::Update()
{

}

void Cloud::FinalUpdate()
{
    _pTransform->Update();
}

void Cloud::Render()
{
    GRenderer->RenderCloud(_pCloudObj, fAnimSpeed, &_pTransform->GetWorldTransform(), fLightAbsorptionCoeff, &vLightDir, fDensityAbsorption, &vLightColor, fAniso);
}

void Cloud::RenderGUI()
{
    ModelObject::RenderGUI();

    std::wstring ModelName = Name + std::wstring(L"_" + std::to_wstring(_uInstanceCount));
    char Title[_MAX_PATH] = {};
    strcpy_s(Title, ToString(ModelName + L" edit").c_str());

    ImGui::Begin(Title);
    ImGui::SliderFloat("Anim Speed", &fAnimSpeed, 0.0f, 0.001f, "%.6f");
    ImGui::SliderFloat("Light Absorption", &fLightAbsorptionCoeff, 0.0f, 10.0f);
    ImGui::SliderFloat3("Light Dir", &vLightDir.x, 0.0f, 1.0f);
    ImGui::SliderFloat("Density Absorption", &fDensityAbsorption, 0.0f, 50.0f);
    ImGui::SliderFloat3("Light Color", &vLightColor.x, 0.0f, 50.0f);
    ImGui::SliderFloat("Aniso", &fAniso, 0.0f, 1.0f);
    ImGui::End();
}

void Cloud::CleanUp()
{
    if (_pCloudObj)
    {
        _pCloudObj->Release();
        _pCloudObj = nullptr;
    }
}
