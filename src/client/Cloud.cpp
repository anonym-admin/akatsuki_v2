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
    GRenderer->RenderCloud(_pCloudObj, 0.0005f, &_pTransform->GetWorldTransform(), fLightAbsorptionCoeff, &vLightDir, fDensityAbsorption, &vLightColor, fAniso);
}

void Cloud::CleanUp()
{
    if (_pCloudObj)
    {
        _pCloudObj->Release();
        _pCloudObj = nullptr;
    }
}
