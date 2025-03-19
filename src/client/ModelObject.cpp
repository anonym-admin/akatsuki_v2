#include "pch.h"
#include "ModelObject.h"
#include "Swat.h"

/*
=================
Model Object
=================
*/

ModelObject::ModelObject()
{
    if (!Initialize())
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

AkBool ModelObject::Initialize()
{
    // Create Model
    Vector3 vExtent = Vector3(1.0f);
    Vector3 vAlbedo = Vector3(1.0f);
    Vector3 vEmissive = Vector3(0.0f);
    CreateCube(&vExtent, &vAlbedo, 1.0f, 0.0f, &vEmissive, nullptr);

    // Create Transform
    _pTransform = CreateTransform();

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

void ModelObject::CreateCube(const Vector3* pExtent, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, const wchar_t** ppTexFimenames)
{
    // Create Model.
    AkU32 uMeshDataNum = 0;
    MeshData_t* pCube = GeometryGenerator::MakeCubeWidthExtent(&uMeshDataNum, pExtent);
    Vector3 vAlbedo = Vector3(1.0f);
    Vector3 vEmissive = Vector3(0.0f);
    
    if (ppTexFimenames)
    {
        for (AkU32 i = 0; i < uMeshDataNum; i++)
        {
            if (ppTexFimenames[i])
            {
                wcscpy_s(pCube[i].wcAlbedoTextureFilename, ppTexFimenames[i]);
            }
        }
    }

    _pModel = CreateModel(pCube, uMeshDataNum, &vAlbedo, 1.0f, 0.0f, &vEmissive, AK_FALSE);

    // Create Collider.
    Vector3 vMin = Vector3(0.0f);
    Vector3 vMax = Vector3(0.0f);
    CalcColliderMinMax(pCube, uMeshDataNum, &vMin, &vMax);
    _pCollider = CreateBoxCollider(&vMin, &vMax);

    GeometryGenerator::DestroyGeometry(pCube, uMeshDataNum);
}

void ModelObject::CleanUp()
{

}
