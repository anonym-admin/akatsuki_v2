#include "pch.h"
#include "Cloud.h"
#include "CloudModel.h"

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

Cloud::Cloud(const Cloud& Other)
{
    // Copy Model.
    _pCloudModel = Other._pCloudModel;
    _pCloudModel->AddRef();

    // Create Transform.
    _pTransform = CreateTransform();

    // Create Culling Collider.
    Vector3 vMin = ((BoxCollider*)Other._pCullingCollider)->GetMinWorld();
    Vector3 vMax = ((BoxCollider*)Other._pCullingCollider)->GetMaxWorld();
    _pCullingCollider = CreateBoxCollider(&vMin, &vMax);
}

Cloud::~Cloud()
{
    CleanUp();
}

AkBool Cloud::Initialize()
{
    // Create Model.
    _pCloudModel = new CloudModel;

    // Create Transform
    _pTransform = CreateTransform();

    // Create Culling Collider.
    Vector3 vMin = Vector3(0.0f);
    Vector3 vMax = Vector3(0.0f);
    _pCloudModel->GetMinMax(&vMin, &vMax);
    _pCullingCollider = CreateBoxCollider(&vMin, &vMax);

    return AK_TRUE;
}

void Cloud::Update()
{

}

void Cloud::FinalUpdate()
{
    _pTransform->Update();

    _pCullingCollider->Update();
}

void Cloud::Render()
{
    _pCloudModel->UpdateWorldRow(&_pTransform->GetWorldTransform());

    _pCloudModel->Render();
}

void Cloud::RenderGUI()
{
    ModelObject::RenderGUI();
    
    _pCloudModel->RenderGUI();
}

Cloud* Cloud::Clone()
{
    return new Cloud(*this);
}

void Cloud::CleanUp()
{
    if (_pCloudModel)
    {
        _pCloudModel->Release();
        _pCloudModel = nullptr;
    }
}
