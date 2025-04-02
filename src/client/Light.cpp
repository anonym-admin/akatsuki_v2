#include "pch.h"
#include "Light.h"

/*
==========
Light
==========
*/

Light::Light(LIGHT_TYPE eType, const Vector3* pRadiance, const Vector3* pPos, AkF32 fRadius, AkBool bShadow)
{
    if (!Initiailize(eType, pRadiance, pPos, fRadius, bShadow))
    {
        __debugbreak();
    }
}

Light::~Light()
{
    CleanUp();
}

AkBool Light::Initiailize(LIGHT_TYPE eType, const Vector3* pRadiance, const Vector3* pPos, AkF32 fRadius, AkBool bShadow)
{
    _eType = eType;
    _vRadiance = *pRadiance;
    _fRadius = fRadius;

    _pTransform = new Transform;

    _pTransform->SetPosition(pPos);

    switch (eType)
    {
    case LIGHT_TYPE::POINT:
        GRenderer->AddPointLight(pRadiance, pPos, fRadius, _fFallOffStart, _fFallOffEnd, bShadow);
        break;
    }

    AkU32 uMeshDataNum = 0;
    MeshData_t* pSquare = GeometryGenerator::MakeSphere(&uMeshDataNum, 0.25f, 8, 8);
    _pMeshObj = GRenderer->CreateBasicMeshObject();
    _pMeshObj->CreateMeshBuffers(pSquare, uMeshDataNum);
    Vector3 vAlbedo = Vector3(0.0f);
    Vector3 vEmissive = Vector3(1.0f, 0.0f, 0.0f);
    _pMeshObj->UpdateMaterialBuffers(&vAlbedo, 0.0f, 0.0f, &vEmissive);
    GeometryGenerator::DestroyGeometry(pSquare, uMeshDataNum);

    return AK_TRUE;
}

void Light::Update()
{
    _pTransform->Update();

    Vector3 vPos = _pTransform->GetPosition();
    GRenderer->UpdatePointLight(0, &_vRadiance, &vPos, _fRadius, _fFallOffStart, _fFallOffEnd, AK_FALSE);
}

void Light::Render()
{
    GRenderer->RenderBasicMeshObject(_pMeshObj, &_pTransform->GetWorldTransform());
}

void Light::RenderGUI()
{
    ImGui::Begin("Light");

    ImGui::InputFloat3("Radiance", &_vRadiance.x);
    ImGui::InputFloat("Radius", &_fRadius);

    ImGui::End();
}

void Light::SetFallOffDistance(AkF32 fStart, AkF32 fEnd)
{
    if (fStart > fEnd)
    {
        _fFallOffStart = fEnd;
        _fFallOffEnd = fStart;
    }

    _fFallOffStart = fStart;
    _fFallOffEnd = fEnd;
}

void Light::CleanUp()
{
    if (_pTransform)
    {
        delete _pTransform;
        _pTransform = nullptr;
    }

    if (_pMeshObj)
    {
        _pMeshObj->Release();
        _pMeshObj = nullptr;
    }
}
