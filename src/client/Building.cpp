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

AkBool UBuilding::Initialize(Application* pApp)
{
	if (!Actor::Initialize(pApp))
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
	// Model �� ��ġ ����
	UpdateModelTransform();

	// Collider ������Ʈ
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

void UBuilding::OnCollision(Collider* pOther)
{
}

void UBuilding::OnCollisionEnter(Collider* pOther)
{
}

void UBuilding::OnCollisionExit(Collider* pOther)
{
}

void UBuilding::CleanUp()
{
}
