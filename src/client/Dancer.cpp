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

AkBool UDancer::Initialize(UApplication* pApp)
{
	if (!UActor::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Bind Model.
	UModelManager* pModelManager = pApp->GetModelManager();
	UModel* pDancerModel = pModelManager->GetModel(MODEL_TYPE::BLENDER_MODEL_DANCER);
	BindModel(pDancerModel);

	// Create Collider.
	UCollider* pCollider = CreateCollider();
	Vector3 vCenter = Vector3(0.0f);
	AkF32 fRadius = 0.5f;
	pCollider->CreateBoundingSphere(fRadius, &vCenter);

	UGravity* pGravity = CreateGravity();
	
	URigidBody* pRigidBody = CreateRigidBody();
	pRigidBody->SetFrictionCoef(0.0f);
	pRigidBody->SetMaxVeleocity(10.0f);

    return AK_TRUE;
}

void UDancer::Update(const AkF32 fDeltaTime)
{
	URigidBody* pRigidBody = GetRigidBody();

	if (!_bGroundCollision)
	{
		// 지면과 충돌하지 않았을때 중력 가속도 적용
		pRigidBody->SetVelocity(0.0f, pRigidBody->GetVelocity().y, 0.0f);
	}
	else
	{
		// 지면과 충돌 시 위치 보정 중력 가속도는 적용되지 않음
		Vector3 vPos = GetPosition();

		pRigidBody->SetVelocity(0.0f, 0.0f, 0.0f);
		SetPosition(vPos.x, vPos.y, vPos.z);
	}

	PlayAnimation(fDeltaTime, L"Dancing.anim");
}

void UDancer::FinalUpdate(const AkF32 fDeltaTime)
{
	UDancerModel* pModel = (UDancerModel*)GetModel(GetModelContextIndex());
	UGravity* pGravity = GetGravity();
	URigidBody* pRigidBody = GetRigidBody();
	UCollider* pCollider = GetCollider();

	// Gravity 업데이트
	if (!_bGroundCollision)
	{
		pGravity->Update(fDeltaTime);
	}

	// Rigid Body 업데이트
	pRigidBody->Update(fDeltaTime);

	// 첫프레임 위치 보정
	Vector3 vPos = GetPosition();

	if (_bFirst)
	{
		SetPosition(vPos.x, 0.5f, vPos.z);

		_bFirst = AK_FALSE;
	}

	// Collider 업데이트
	pCollider->Update(fDeltaTime);

	// Model 의 위치 변경
	UpdateModelTransform();
}

void UDancer::RenderShadow()
{
	RenderShadowOfModel();
}

void UDancer::Render()
{
	UApplication* pApp = GetApp();

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
		UCollider* pCollider = GetCollider();

		pCollider->Render();
	}
}

void UDancer::OnCollision(UCollider* pOther)
{
	UActor* pOwner = pOther->GetOwner();
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

void UDancer::OnCollisionEnter(UCollider* pOther)
{
	UActor* pOwner = pOther->GetOwner();
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

void UDancer::OnCollisionExit(UCollider* pOther)
{
}

void UDancer::CleanUp()
{
	DestroyCollider();

	DesteoyRigidBody();

	DestroyGravity();
}
