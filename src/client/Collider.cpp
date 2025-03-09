#include "pch.h"
#include "Collider.h"
#include "Actor.h"
#include "GeometryGenerator.h"
#include "RigidBody.h"
#include "Timer.h"
#include "KDTree.h"
#include "Transform.h"

/*
===========
Collider
===========
*/

AkU32 Collider::sm_uID;

Collider::Collider(Actor* pOwner)
{
	if (!Initialize(pOwner))
	{
		__debugbreak();
	}
}

Collider::~Collider()
{
	CleanUp();
}

AkBool Collider::Initialize(Actor* pOwner)
{
	_pOwner = pOwner;

	_uID = sm_uID++;

	return AK_TRUE;
}

void Collider::Update()
{
	Vector3 vOwnerPos = _pOwner->GetTransform()->GetPosition();

	if (_pBox)
	{
		Vector3 vExtent = _pBox->vMax - _pBox->vMin;

		_pBox->vMin = -vExtent * 0.5f + vOwnerPos;
		_pBox->vMax = vExtent * 0.5f + vOwnerPos;
	}
	else
	{
		_pSphere->vCenter = vOwnerPos;
	}

	_mWorldRow = Matrix::CreateTranslation(vOwnerPos);
}

void Collider::Render()
{
	if (_pMeshObj)
	{
		_pRenderer->RenderBasicMeshObject(_pMeshObj, &_mWorldRow);
	}
}

void Collider::CreateBoundingBox(const Vector3* pMin, const Vector3* pMax)
{
	_pBox = new AkBox_t;
	_pBox->vMin = *pMin;
	_pBox->vMax = *pMax;

	_eShapeType = COLLIDER_SHAPE_TYPE::COLLIDER_SHAPE_BOX;

	if (_pRenderer)
	{
		AkF32 fWidth = abs(_pBox->vMax.x - _pBox->vMin.x);
		AkF32 fHeight = abs(_pBox->vMax.y - _pBox->vMin.y);
		AkF32 fDepth = abs(_pBox->vMax.z - _pBox->vMin.z);
		Vector3 pExtent = Vector3(fWidth, fHeight, fDepth) * 0.5f;
		AkU32 uMeshDataNum = 0;
		MeshData_t* pBoxMeshData = GeometryGenerator::MakeCubeWidthExtent(&uMeshDataNum, &pExtent);

		Vector3 vAlbedo = Vector3(0.0f);
		AkF32 fMetallic = 0.0f;
		AkF32 fRoughness = 0.0f;
		Vector3 vEmissive = Vector3(0.0f, 1.0f, 0.0f);
		_pMeshObj = _pRenderer->CreateBasicMeshObject();
		_pMeshObj->CreateMeshBuffers(pBoxMeshData, uMeshDataNum);
		_pMeshObj->EnableWireFrame();
		_pMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmissive);

		GeometryGenerator::DestroyGeometry(pBoxMeshData, uMeshDataNum);
	}
}

void Collider::CreateBoundingSphere(AkF32 fRadius, const Vector3* pCenter)
{
	_pSphere = new AkSphere_t;
	_pSphere->fRadius = fRadius;
	_pSphere->vCenter = *pCenter;

	_eShapeType = COLLIDER_SHAPE_TYPE::COLLIDER_SHAPE_SPHERE;

	if (_pRenderer)
	{
		AkU32 uMeshDataNum = 0;
		MeshData_t* pSphereMeshData = GeometryGenerator::MakeSphere(&uMeshDataNum, fRadius, 16, 16);

		Vector3 vAlbedo = Vector3(0.0f);
		AkF32 fMetallic = 0.0f;
		AkF32 fRoughness = 0.0f;
		Vector3 vEmissive = Vector3(0.0f, 1.0f, 0.0f);
		_pMeshObj = _pRenderer->CreateBasicMeshObject();
		_pMeshObj->CreateMeshBuffers(pSphereMeshData, uMeshDataNum);
		_pMeshObj->EnableWireFrame();
		_pMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmissive);

		GeometryGenerator::DestroyGeometry(pSphereMeshData, uMeshDataNum);
	}
}

void Collider::CreateTriangle(const Vector3* pV0, const Vector3* pV1, const Vector3* pV2)
{
	_pTriangle = new AkTriangle_t;

	_pTriangle->vP[0] = *pV0;
	_pTriangle->vP[1] = *pV1;
	_pTriangle->vP[2] = *pV2;

	_pTriangle->vNormal = (*pV1 - *pV0).Cross(*pV2 - *pV0);
	_pTriangle->vNormal.Normalize();
}

void Collider::DestroyBoundingBox()
{
	if (_pBox)
	{
		delete _pBox;
		_pBox = nullptr;
	}
}

void Collider::DestroyBoundingSphere()
{
	if (_pSphere)
	{
		delete _pSphere;
		_pSphere = nullptr;
	}
}

void Collider::DestroyTriangle()
{
	if (_pTriangle)
	{
		delete _pTriangle;
		_pTriangle = nullptr;
	}
}

void Collider::OnCollision(Collider* pCollider)
{
	_pOwner->OnCollision(pCollider);
}

void Collider::OnCollisionEnter(Collider* pCollider)
{
	_pOwner->OnCollisionEnter(pCollider);
}

void Collider::OnCollisionExit(Collider* pCollider)
{
	_pOwner->OnCollisionExit(pCollider);
}

void Collider::CleanUp()
{
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}

	DestroyBoundingBox();
	DestroyBoundingSphere();
	DestroyTriangle();
}





/*
===========
New
===========
*/

AkBool New_Collider::DRAW_COLLIDER;

New_Collider::New_Collider()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

New_Collider::~New_Collider()
{
	CleanUp();
}

AkBool New_Collider::Initialize()
{
	_pTransform = new Transform;

	return AK_TRUE;
}

void New_Collider::Update()
{
	_pTransform->Update();
}

void New_Collider::Render()
{
	if (!_pLineObj)
		return;

	if (!DRAW_COLLIDER)
		return;

	GRenderer->RenderLineObject(_pLineObj, &_pTransform->GetWorldTransform());
}

void New_Collider::CleanUp()
{
	if (_pTransform)
	{
		delete _pTransform;
		_pTransform = nullptr;
	}
	if (_pLineObj)
	{
		_pLineObj->Release();
		_pLineObj = nullptr;
	}
}
