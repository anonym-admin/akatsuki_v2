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
