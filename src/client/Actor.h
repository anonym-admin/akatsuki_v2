#pragma once

#include "BaseObject.h"

/*
=========
Actor
=========
*/

class Collider;
class RigidBody;
class Gravity;
class Camera;
class Weapon;
class Animation;

class Actor : public BaseObject
{
public:
	virtual ~Actor();

	virtual void Update() = 0;
	virtual void FinalUpdate() = 0;
	virtual void RenderShadow() = 0;
	virtual void Render() = 0;

	virtual void OnCollision(class Collider* pOther) = 0;
	virtual void OnCollisionEnter(class  Collider* pOther) = 0;
	virtual void OnCollisionExit(class Collider* pOther) = 0;

	Collider* CreateBoxCollider(const Vector3* pMin = nullptr, const Vector3* pMax = nullptr, const Vector3* pColor = nullptr);
	Collider* CreateSphereCollider(AkF32 fRadius = 0.5f, AkU32 uStack = 16, AkU32 uSlice = 32, const Vector3* pColor = nullptr);
	Collider* CreateCapsuleCollider(AkF32 fRadius = 0.5f, AkF32 fHeight = 1.0f, AkU32 uStack = 16, AkU32 uSlice = 32, const Vector3* pColor = nullptr);
	RigidBody* CreateRigidBody();
	Gravity* CreateGravity();
	Camera* CreateCamera(const Vector3* pPos, const Vector3* pYawPitchRoll = nullptr);
	Animation* CreateAnimation(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum);

	void DestroyCollider();
	void DesteoyRigidBody();
	void DestroyGravity();
	void DestroyCamera();
	void DestroyAnimation();

	Collider* GetCollider() { return _pCollider; }
	RigidBody* GetRigidBody() { return _pRigidBody; }
	Gravity* GetGravity() { return _pGravity; }
	Camera* GetCamera() { return _pCamera; }
	Animation* GetAnimation() { return _pAnimation; }

	void BindAnimation(Animation* pAnim);
	void UnBindAnimation();

	void SetWeapon(Weapon* pWeapon);

	List_t tLink;

private:
	void CleanUp();

protected:
	// 충돌체
	Collider* _pCollider = nullptr;

	// 강체
	RigidBody* _pRigidBody = nullptr;

	// 중력
	Gravity* _pGravity = nullptr;

	// 카메라
	Camera* _pCamera = nullptr;

	// 무기
	Weapon* _pWeapon = nullptr;

	// 애니메이션
	Animation* _pAnimation = nullptr;

public:
	AkBool GroundCollision = AK_FALSE;
	AkBool SpeedUp = AK_FALSE;
	AkBool Jumping = AK_FALSE;
	AkBool BindWeapon = AK_FALSE;
	AkBool First = AK_TRUE;
	AkBool RightHand = AK_FALSE;
	AkBool LeftHand = AK_FALSE;
	AkBool Fire = AK_FALSE;
	AkBool DrawNormal = AK_FALSE;
};
