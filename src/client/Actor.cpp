#include "pch.h"
#include "Actor.h"
#include "PlayerModel.h"
#include "GameInput.h"
#include "Application.h"
#include "RigidBody.h"
#include "Gravity.h"
#include "Camera.h"
#include "Collider.h"

UActor::UActor()
{
}

UActor::~UActor()
{
	CleanUp();
}

AkBool UActor::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	return AK_TRUE;
}

void UActor::BindModel(UModel* pModel)
{
	_pModel = pModel;
	_eModelCxtIndex = (MODEL_CONTEXT_INDEX)_pModel->AddRef();
}

void UActor::SetScale(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vScale = Vector3(fX, fY, fZ);
}

void UActor::SetRotationX(AkF32 vRot)
{
	_vRot.x = vRot;
}

void UActor::SetRotationY(AkF32 vRot)
{
	_vRot.y = vRot;
}

void UActor::SetRotationZ(AkF32 vRot)
{
	_vRot.z = vRot;
}

void UActor::SetQuaternion(Quaternion qQuat)
{
	_qQuat = qQuat;
}

void UActor::SetPosition(AkF32 fX, AkF32 fY, AkF32 fZ)
{
	_vPos = Vector3(fX, fY, fZ);
}

void UActor::SetName(const wchar_t* wcName)
{
	wcscpy_s(_wcName, wcName);
}

UModel* UActor::GetModel(MODEL_CONTEXT_INDEX eModelCtxIndex)
{
	_pModel->SetModelContextTableIndex((AkU32)eModelCtxIndex);

	return _pModel;
}

void UActor::Update(const AkF32 fDeltaTime)
{
}

void UActor::Render()
{
}

UCollider* UActor::CreateCollider()
{
	_pCollider = new UCollider;
	_pCollider->Initialize(this, _pApp->GetRenderer());
	return _pCollider;
}

URigidBody* UActor::CreateRigidBody()
{
	_pRigidBody = new URigidBody;
	_pRigidBody->Initialize(this);
	return _pRigidBody;
}

UGravity* UActor::CreateGravity()
{
	_pGravity = new UGravity;
	_pGravity->Initialize(this);
	return _pGravity;
}

UInGameCamera* UActor::CreateCamera()
{
	_pCamera = new UInGameCamera;
	_pCamera->Initialize(_pApp, this, Vector3(0.0f, 0.5f, -2.0f));
	return _pCamera;
}

void UActor::DestroyCollider()
{
	if (_pCollider)
	{
		delete _pCollider;
		_pCollider = nullptr;
	}
}

void UActor::DesteoyRigidBody()
{
	if (_pRigidBody)
	{
		delete _pRigidBody;
		_pRigidBody = nullptr;
	}
}

void UActor::DestroyGravity()
{
	if (_pGravity)
	{
		delete _pGravity;
		_pGravity = nullptr;
	}
}

void UActor::DestroyCamera()
{
	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}
}

AkBool UActor::PlayAnimation(const AkF32 fDeltaTime, const wchar_t* wcClipName, AkBool bInPlace)
{
	AkBool bIsEnd = AK_FALSE;

	UModel* pModel = (UModel*)GetModel(_eModelCxtIndex);

	// 애니메이션이 빨라지는 이슈 발생 => 임시 방안 코드 작성 => 해결책은 TODO!!
	if (MODEL_CONTEXT_INDEX::MODEL_CONTEXT_0 == _eModelCxtIndex)
	{
		bIsEnd = pModel->PlayAnimation(fDeltaTime, wcClipName, bInPlace);
	}

	return bIsEnd;
}

void UActor::CleanUp()
{
	UnBindModel();
}

void UActor::UnBindModel()
{
	if (_pModel)
	{
		_pModel->Release();
		_pModel = nullptr;
	}
}

void UActor::UpdateModelTransform()
{
	UModel* pModel = (UModel*)GetModel(_eModelCxtIndex);

	Matrix mWorld = Matrix();

	// 우선, 쿼터니언은 제외하고 코드 작성
	mWorld = Matrix::CreateScale(_vScale) * Matrix::CreateRotationX(_vRot.x) * Matrix::CreateRotationY(_vRot.y) * Matrix::CreateRotationZ(_vRot.z) * Matrix::CreateTranslation(_vPos);

	pModel->SetWorldMatrix((AkU32)_eModelCxtIndex , &mWorld);
}

void UActor::RenderShadowOfModel()
{
	UModel* pModel = GetModel(_eModelCxtIndex);

	pModel->RenderShadow();
}

void UActor::RenderModel()
{
	UModel* pModel = GetModel(_eModelCxtIndex);

	pModel->Render();
}

void UActor::RenderNormal()
{
	UModel* pModel = GetModel(_eModelCxtIndex);

	pModel->RenderNormal();
}






