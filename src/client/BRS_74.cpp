#include "pch.h"
#include "BRS_74.h"
#include "BRS_74_Model.h"
#include "Collider.h"
#include "Application.h"
#include "EventManager.h"
#include "ModelManager.h"
#include "Player.h"

/*
=============
BRS_74
=============
*/

UBRS_74::UBRS_74()
{
}

UBRS_74::~UBRS_74()
{
	CleanUp();
}

AkBool UBRS_74::Initialize(UApplication* pApp)
{
	return AK_FALSE;
}

AkBool UBRS_74::Initialize(UApplication* pApp, const Vector3* pExtent, const Vector3* pCenter)
{
	if (!UWeapon::Initialize(pApp, pExtent, pCenter))
	{
		__debugbreak();
		return AK_FALSE;
	}

	// Bind Model.
	UModelManager* pModelManager = pApp->GetModelManager();
	UBRS_74_Model* pBRSModel = (UBRS_74_Model*)pModelManager->GetModel(MODEL_TYPE::BRS_74);
	BindModel(pBRSModel);

	// Create Bounding Box.
	UCollider* pCollider = GetCollider();
	Vector3 vMin = Vector3(0.0f);
	Vector3 vMax = Vector3(0.0f);
	pBRSModel->GetMinMaxPosition(&vMin, &vMax);
	pCollider->CreateBoundingBox(&vMin, &vMax);

	return AK_TRUE;
}

void UBRS_74::Update(const AkF32 fDeltaTime)
{
	UWeapon::Update(fDeltaTime);
}

void UBRS_74::FinalUpdate(const AkF32 fDeltaTime)
{
	UWeapon::FinalUpdate(fDeltaTime);
}

void UBRS_74::RenderShadow()
{
	RenderShadowOfModel();
}

void UBRS_74::Render()
{
	IRenderer* pRenderer = GetApp()->GetRenderer();

	UWeapon::Render();
}

void UBRS_74::OnCollision(UCollider* pOther)
{
}

void UBRS_74::OnCollisionEnter(UCollider* pOther)
{
	UApplication* pApp = GetApp();
	UGameEventManager* pGameEventManager = pApp->GetGameEventManager();
	IRenderer* pRenderer = pApp->GetRenderer();
	UActor* pActor = pOther->GetOwner();
	const wchar_t* wcName = pActor->GetName();

	if (!wcscmp(wcName, L"Player"))
	{
		// Bind Weapon.
		UPlayer* pPlayer = (UPlayer*)pActor;
		pPlayer->BindWeapon(this);
	}
}

void UBRS_74::OnCollisionExit(UCollider* pOther)
{
}

void UBRS_74::CleanUp()
{

}

