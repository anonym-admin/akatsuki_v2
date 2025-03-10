#include "pch.h"
#include "Container.h"
#include "Transform.h"
#include "Model.h"
#include "Collider.h"
#include "Swat.h"

Container::Container()
{
    if (!Initailize())
    {
        __debugbreak();
    }
}

Container::~Container()
{
    CleanUp();
}

AkBool Container::Initailize()
{
    // Create Model
    AkU32 uMeshDataNum = 0;
    MeshData_t* pCube = GeometryGenerator::MakeCube(&uMeshDataNum, 0.5f);
    Vector3 vAlbedo = Vector3(1.0f);
    Vector3 vEmissvie = Vector3(0.0f);
    _pModel = CreateModel(pCube, uMeshDataNum, &vAlbedo, 1.0f, 0.0f, &vEmissvie, AK_FALSE);
    GeometryGenerator::DestroyGeometry(pCube, uMeshDataNum);

    // Create Transform
    _pTransform = CreateTransform();

    // Create Collider.
    Vector3 vMin = Vector3(-0.5f);
    Vector3 vMax = Vector3(0.5f);
    _pCollider = CreateBoxCollider(&vMin, &vMax);

    return AK_TRUE;
}

void Container::Update()
{
}

void Container::FinalUpdate()
{
    _pTransform->Update();

    _pCollider->Update();

    _pModel->UpdateWorldRow(_pTransform->GetWorldTransformAddr());
}

void Container::Render()
{
    _pModel->Render();

    _pCollider->Render();
}

void Container::RenderShadow()
{
    _pModel->RenderShadow();
}

void Container::OnCollisionEnter(Collider* pOther)
{
    Actor* pOtherOwner = pOther->GetOwner();
    if (!wcscmp(pOtherOwner->Name, L"Swat"))
    {
        ((Swat*)pOtherOwner)->ActionReaction(_pCollider);
    }
}

void Container::OnCollision(Collider* pOther)
{
    Actor* pOtherOwner = pOther->GetOwner();
    if (!wcscmp(pOtherOwner->Name, L"Swat"))
    {
        ((Swat*)pOtherOwner)->ActionReaction(_pCollider);
    }
}

void Container::OnCollisionExit(Collider* pOther)
{
}

Container* Container::Clone()
{
    Spawn::Clone();
    return new Container();
}

void Container::CleanUp()
{
}
