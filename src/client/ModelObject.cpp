#include "pch.h"
#include "ModelObject.h"
#include "Swat.h"

/*
=================
Model Object
=================
*/

ModelObject::ModelObject(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
    if (!Initialize(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive))
    {
        __debugbreak();
    }
}

ModelObject::ModelObject(const wchar_t* wcFilename)
{
    if (!Initialize(wcFilename))
    {
        __debugbreak();
    }
}

ModelObject::~ModelObject()
{
    CleanUp();
}

AkBool ModelObject::Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
    // Create Model
    _pModel = CreateModel(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive, AK_FALSE);

    // Create Transform
    _pTransform = CreateTransform();

    // Create Collider.
    Vector3 vMin = Vector3(-0.5f);
    Vector3 vMax = Vector3(0.5f);
    CalcColliderMinMax(pMeshData, uMeshDataNum, &vMin, &vMax);
    _pCollider = CreateBoxCollider(&vMin, &vMax);

    return AK_TRUE;
}

AkBool ModelObject::Initialize(const wchar_t* wcFilename)
{
    AkU32 uMeshDataNum = 0;
    MeshData_t* pMeshData = GeometryGenerator::ReadFromFile(&uMeshDataNum, MODEL_FILE_PATH, wcFilename, AK_FALSE);
    Vector3 vAlbedo = Vector3(1.0f);
    Vector3 vEmissive = Vector3(0.0f);
    _pModel = CreateModel(pMeshData, uMeshDataNum, &vAlbedo, 1.0f, 0.0f, &vEmissive, AK_FALSE);

    // Create Transform
    _pTransform = CreateTransform();

    // Create Collider.
    Vector3 vMin = Vector3(-0.5f);
    Vector3 vMax = Vector3(0.5f);
    CalcColliderMinMax(pMeshData, uMeshDataNum, &vMin, &vMax);
    _pCollider = CreateBoxCollider(&vMin, &vMax);

    return AK_TRUE;
}

void ModelObject::Update()
{
}

void ModelObject::FinalUpdate()
{
    _pTransform->Update();

    _pCollider->Update();

    _pModel->UpdateWorldRow(_pTransform->GetWorldTransformAddr());
}

void ModelObject::Render()
{
    _pModel->Render();

    _pCollider->Render();
}

void ModelObject::RenderShadow()
{
    _pModel->RenderShadow();
}

void ModelObject::OnCollisionEnter(Collider* pOther)
{
    Actor* pOtherOwner = pOther->GetOwner();
    if (!wcscmp(pOtherOwner->Name, L"Swat"))
    {
        ((Swat*)pOtherOwner)->ActionReaction(_pCollider);
    }
}

void ModelObject::OnCollision(Collider* pOther)
{
    Actor* pOtherOwner = pOther->GetOwner();
    if (!wcscmp(pOtherOwner->Name, L"Swat"))
    {
        ((Swat*)pOtherOwner)->ActionReaction(_pCollider);
    }
}

void ModelObject::OnCollisionExit(Collider* pOther)
{
}

ModelObject* ModelObject::Clone()
{
    Spawn::Clone();
    return new ModelObject();
}

void ModelObject::CleanUp()
{

}
