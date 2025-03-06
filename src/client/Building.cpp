#include "pch.h"
#include "Building.h"
#include "Player.h"
#include "RigidBody.h"
#include "PlayerModel.h"
#include "Collider.h"

UBuilding::UBuilding()
{
}

UBuilding::~UBuilding()
{
	CleanUp();
}

AkBool UBuilding::Initialize(UApplication* pApp)
{
	if (!UActor::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}
	return AK_TRUE;
}

void UBuilding::Update(const AkF32 fDeltaTime)
{
}

void UBuilding::FinalUpdate(const AkF32 fDeltaTime)
{
	// Model 의 위치 변경
	UpdateModelTransform();

	// Collider 업데이트
	// pCollider->Update(fDeltaTime);
}

void UBuilding::Render()
{
	// Render Model.
	RenderModel();

	// Render Normal.
	if (IsDrawNoraml())
	{
		RenderNormal();
	}
}

void UBuilding::OnCollision(UCollider* pOther)
{
}

void UBuilding::OnCollisionEnter(UCollider* pOther)
{
}

void UBuilding::OnCollisionExit(UCollider* pOther)
{
}

void UBuilding::CleanUp()
{
}
