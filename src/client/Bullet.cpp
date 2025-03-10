#include "pch.h"
#include "Bullet.h"
#include "Weapon.h"

Bullet::Bullet(Weapon* pOwner)
{
	if (!Initailize(pOwner))
	{
		__debugbreak();
	}
}

Bullet::~Bullet()
{
}

AkBool Bullet::Initailize(Weapon* pOwner)
{
	// Set Owner
	_pOwner = pOwner;

	// Create Moodel
	AssetMeshDataContainer_t* pMeshDataContainer = GAssetManager->GetMeshDataContainer(ASSET_MESH_DATA_TYPE::BULLET);
	Vector3 vAlbedo = Vector3(1.0f);
	Vector3 vEmissvie = Vector3(0.0f);
	_pModel = CreateModel(pMeshDataContainer, &vAlbedo, 1.0f, 0.0f, &vEmissvie, AK_FALSE);

	// Create Transform
	_pTransform = CreateTransform();

	// Create Collider
	Vector3 vMin = Vector3(-0.5f);
	Vector3 vMax = Vector3(0.5f);
	CalcColliderMinMax(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum, &vMin, &vMax);
	_pCollider = CreateBoxCollider(&vMin, &vMax);

	// Delete MeshData
	// GAssetManager->DeleteMeshData(ASSET_MESH_DATA_TYPE::BULLET);

	// Create RigidBody
	_pRigidBody = CreateRigidBody();
	_pRigidBody->SetMaxVeleocity(100.0f);
	_pRigidBody->SetFrictionCoef(0.0f);

	return AK_TRUE;
}

void Bullet::Update()
{

}

void Bullet::FinalUpdate()
{
	_pRigidBody->Update();

	_pTransform->SetParent(&_pOwner->GetTransform()->GetWorldTransform());

	_pTransform->Update();

	printf("%lf %Lf %lf\n", _pTransform->GetPosition().x, _pTransform->GetPosition().y, _pTransform->GetPosition().z);

	_pCollider->Update();

	_pModel->UpdateWorldRow(_pTransform->GetWorldTransformAddr());
}

void Bullet::Render()
{
	_pModel->Render();

	_pCollider->Render();
}

void Bullet::RenderShadow()
{
	_pModel->RenderShadow();
}

void Bullet::OnCollisionEnter(Collider* pOther)
{
}

void Bullet::OnCollision(Collider* pOther)
{
}

void Bullet::OnCollisionExit(Collider* pOther)
{
}

Bullet* Bullet::Clone()
{
	Spawn::Clone();
	return new Bullet(*this);
}

void Bullet::CleanUp()
{
}
