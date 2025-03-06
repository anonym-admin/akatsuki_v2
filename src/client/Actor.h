#pragma once

enum class MODEL_CONTEXT_INDEX
{
	MODEL_CONTEXT_0,
	MODEL_CONTEXT_1,
	MODEL_CONTEXT_2,
	MODEL_CONTEXT_3,
};

/*
=========
Actor
=========
*/

class UApplication;
class UModel;
class UCollider;
class URigidBody;
class UGravity;
class UInGameCamera;

class UActor
{
public:
	UActor();
	virtual ~UActor();

	virtual AkBool Initialize(UApplication* pApp) = 0;
	virtual void Update(const AkF32 fDeltaTime) = 0;
	virtual void FinalUpdate(const AkF32 fDeltaTime) = 0;
	virtual void RenderShadow() = 0;
	virtual void Render() = 0;

	virtual void OnCollision(UCollider* pOther) = 0;
	virtual void OnCollisionEnter(UCollider* pOther) = 0;
	virtual void OnCollisionExit(UCollider* pOther) = 0;

	UCollider* CreateCollider();
	URigidBody* CreateRigidBody();
	UGravity* CreateGravity();
	UInGameCamera* CreateCamera();
	void DestroyCollider();
	void DesteoyRigidBody();
	void DestroyGravity();
	void DestroyCamera();

	// 씬 또는 엔진에서 초기화된 드로우 오브젝트를 설정
	void BindModel(UModel* pModel);
	void SetScale(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetRotationX(AkF32 vRot);
	void SetRotationY(AkF32 vRot);
	void SetRotationZ(AkF32 vRot);
	void SetQuaternion(Quaternion qQuat);
	void SetPosition(AkF32 fX, AkF32 fY, AkF32 fZ);
	void SetGroundCollision(AkBool bIsCollision) { _bGroundCollision = bIsCollision; }
	void SetName(const wchar_t* wcName);
	void SetDrawNormalFlag(AkBool bDrawNormal) { _bDrawNormal = bDrawNormal; }
	AkF32 GetRotationX() { return _vRot.x; }
	AkF32 GetRotationY() { return _vRot.y; }
	AkF32 GetRotationZ() { return _vRot.z; }
	Quaternion GetQuternion() { return _qQuat; }
	Vector3 GetPosition() { return _vPos; }
	MODEL_CONTEXT_INDEX GetModelContextIndex() { return _eModelCxtIndex; }
	UModel* GetModel(MODEL_CONTEXT_INDEX eModelCtxIndex);
	UCollider* GetCollider() { return _pCollider; }
	URigidBody* GetRigidBody() { return _pRigidBody; }
	UGravity* GetGravity() { return _pGravity; }
	UInGameCamera* GetCamera() { return _pCamera; }
	UApplication* GetApp() { return _pApp; }
	const wchar_t* GetName() { return _wcName; }
	AkBool IsDrawNoraml() { return _bDrawNormal; }

	List_t tLink;

protected:
	AkBool PlayAnimation(const AkF32 fDeltaTime, const wchar_t* wcClipName, AkBool bInPlace = AK_FALSE);
	virtual void UpdateModelTransform();
	void RenderShadowOfModel();
	void RenderModel();
	void RenderNormal();

private:
	virtual void CleanUp() = 0;
	void UnBindModel();

protected:
	AkBool _bGroundCollision = AK_FALSE;
	// 위치, 스케일, 회전 정보
	Vector3 _vScale = Vector3(1.0f);
	Vector3 _vRot = Vector3(0.0f);
	Vector3 _vPos = Vector3(0.0f);
	Quaternion _qQuat = Quaternion();

private:
	UApplication* _pApp = nullptr;

	// 랜더링 하기위한 오브젝트
	UModel* _pModel = nullptr;
	MODEL_CONTEXT_INDEX _eModelCxtIndex = {};

	// 충돌체
	UCollider* _pCollider = nullptr;

	// 강체
	URigidBody* _pRigidBody = nullptr;

	// 중력
	UGravity* _pGravity = nullptr;

	// 카메라
	UInGameCamera* _pCamera = nullptr;

	// name
	wchar_t _wcName[32] = {};

	// Draw Normal Flag
	AkBool _bDrawNormal = AK_FALSE;
};
