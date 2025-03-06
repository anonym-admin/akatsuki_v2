#include "pch.h"
#include "Dancer.h"
#include "DancerModel.h"
#include "Collider.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Application.h"
#include "ModelManager.h"

UDancer::UDancer()
{
}

UDancer::~UDancer()
{
	CleanUp();
}

AkBool UDancer::Initialize(Application* pApp)
{
	if (!Actor::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Bind Model.
	ModelManager* pModelManager = pApp->GetModelManager();
	UModel* pDancerModel = pModelManager->GetModel(MODEL_TYPE::BLENDER_MODEL_DANCER);
	BindModel(pDancerModel);

	// Create Collider.
	Collider* pCollider = CreateCollider();
	Vector3 vCenter = Vector3(0.0f);
	AkF32 fRadius = 0.5f;
	pCollider->CreateBoundingSphere(fRadius, &vCenter);

	Gravity* pGravity = CreateGravity();
	
	RigidBody* pRigidBody = CreateRigidBody();
	pRigidBody->SetFrictionCoef(0.0f);
	pRigidBody->SetMaxVeleocity(10.0f);

    return AK_TRUE;
}

void UDancer::Update(const AkF32 fDeltaTime)
{
	RigidBody* pRigidBody = GetRigidBody();

	if (!_bGroundCollision)
	{
		// ����� �浹���� �ʾ����� �߷� ���ӵ� ����
		pRigidBody->SetVelocity(0.0f, pRigidBody->GetVelocity().y, 0.0f);
	}
	else
	{
		// ����� �浹 �� ��ġ ���� �߷� ���ӵ��� ������� ����
		Vector3 vPos = GetPosition();

		pRigidBody->SetVelocity(0.0f, 0.0f, 0.0f);
		SetPosition(vPos.x, vPos.y, vPos.z);
	}

	PlayAnimation(fDeltaTime, L"Dancing.anim");
}

void UDancer::FinalUpdate(const AkF32 fDeltaTime)
{
	UDancerModel* pModel = (UDancerModel*)GetModel(GetModelContextIndex());
	Gravity* pGravity = GetGravity();
	RigidBody* pRigidBody = GetRigidBody();
	Collider* pCollider = GetCollider();

	// Gravity ������Ʈ
	if (!_bGroundCollision)
	{
		pGravity->Update(fDeltaTime);
	}

	// Rigid Body ������Ʈ
	pRigidBody->Update(fDeltaTime);

	// ù������ ��ġ ����
	Vector3 vPos = GetPosition();

	if (_bFirst)
	{
		SetPosition(vPos.x, 0.5f, vPos.z);

		_bFirst = AK_FALSE;
	}

	// Collider ������Ʈ
	pCollider->Update(fDeltaTime);

	// Model �� ��ġ ����
	UpdateModelTransform();
}

void UDancer::RenderShadow()
{
	RenderShadowOfModel();
}

void UDancer::Render()
{
	Application* pApp = GetApp();

	// Render Model.
	RenderModel();

	// Render Normal.
	if (IsDrawNoraml())
	{
		RenderNormal();
	}

	// Render Collider.
	if (pApp->EnableEditor())
	{
		Collider* pCollider = GetCollider();

		pCollider->Render();
	}
}

void UDancer::OnCollision(Collider* pOther)
{
	Actor* pOwner = pOther->GetOwner();
	const wchar_t* wcName = pOwner->GetName();

	if (!wcscmp(L"Map", wcName))
	{
		AkTriangle_t* pTri = pOther->GetTriangle();
		AkF32 fNdotY = pTri->vNormal.Dot(Vector3(0.0f, 1.0f, 0.0f));
		if (0.0f <= fNdotY - 1.0f && fNdotY - 1.0f <= 1e-5f)
		{
			SetGroundCollision(AK_TRUE);
		}

		AkSphere_t* pSphere = GetCollider()->GetBoundingSphere();
		Vector3 vTriToSphere = pSphere->vCenter - pTri->vP[0];
		AkF32 fDistance = vTriToSphere.Dot(pTri->vNormal);
		Vector3 vProjectedCenter = pSphere->vCenter - pTri->vNormal * (fDistance - pSphere->fRadius);

		SetPosition(vProjectedCenter.x, vProjectedCenter.y, vProjectedCenter.z);
	}
}

void UDancer::OnCollisionEnter(Collider* pOther)
{
	Actor* pOwner = pOther->GetOwner();
	const wchar_t* wcName = pOwner->GetName();

	if (!wcscmp(L"Map", wcName))
	{
		AkTriangle_t* pTri = pOther->GetTriangle();
		AkF32 fNdotY = pTri->vNormal.Dot(Vector3(0.0f, 1.0f, 0.0f));
		if (0.0f <= fNdotY - 1.0f && fNdotY - 1.0f <= 1e-5f)
		{
			SetGroundCollision(AK_TRUE);
		}

		AkSphere_t* pSphere = GetCollider()->GetBoundingSphere();
		Vector3 vTriToSphere = pSphere->vCenter - pTri->vP[0];
		AkF32 fDistance = vTriToSphere.Dot(pTri->vNormal);
		Vector3 vProjectedCenter = pSphere->vCenter - pTri->vNormal * (fDistance - pSphere->fRadius);

		SetPosition(vProjectedCenter.x, vProjectedCenter.y, vProjectedCenter.z);
	}
}

void UDancer::OnCollisionExit(Collider* pOther)
{
}

void UDancer::CleanUp()
{
	DestroyCollider();

	DesteoyRigidBody();

	DestroyGravity();
}
