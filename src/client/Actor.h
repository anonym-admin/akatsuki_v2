#pragma once

#include "BaseObject.h"

enum class ANIM_STATE
{
	PLAYER_ANIM_STATE_IDLE,
	PLAYER_ANIM_STATE_WALK,
	PLAYER_ANIM_STATE_RUN,
	PLAYER_ANIM_STATE_JUMP,
	PLAYER_ANIM_STATE_RIFLE_RUN,
	PLAYER_ANIM_STATE_RIFLE_WALK,
	PLAYER_ANIM_STATE_RIFLE_IDLE,
	PLAYER_ANIM_STATE_RIFLE_RUN_FIRE,
	PLAYER_ANIM_STATE_RIFLE_WALK_FIRE,
	PLAYER_ANIM_STATE_RIFLE_IDLE_FIRE,
	PLAYER_ANIM_STATE_COUNT = 32,
};

enum class MOVE_STATE
{
	PLAYER_MOVE_STATE_NONE,

	PLAYER_MOVE_STATE_FRONT_WALK,
	PLAYER_MOVE_STATE_BACK_WALK,
	PLAYER_MOVE_STATE_RIGHT_WALK,
	PLAYER_MOVE_STATE_LEFT_WALK,

	PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_WALK,
	PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_WALK,
	PLAYER_MOVE_STATE_DIAGONAL_RIHGT_BACK_WALK,
	PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_WALK,

	PLAYER_MOVE_STATE_FRONT_RUN,
	PLAYER_MOVE_STATE_BACK_RUN,
	PLAYER_MOVE_STATE_RIGHT_RUN,
	PLAYER_MOVE_STATE_LEFT_RUN,

	PLAYER_MOVE_STATE_DIAGONAL_RIGHT_FRONT_RUN,
	PLAYER_MOVE_STATE_DIAGONAL_LEFT_FRONT_RUN,
	PLAYER_MOVE_STATE_DIAGONAL_RIGHT_BACK_RUN,
	PLAYER_MOVE_STATE_DIAGONAL_LEFT_BACK_RUN,

	PLAYER_MOVE_STATE_COUNT,
};

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

class Actor : public BaseObject
{
public:
	virtual ~Actor();

	virtual void Update() = 0;
	virtual void FinalUpdate() = 0;
	virtual void RenderShadow() = 0;
	virtual void Render() = 0;

	virtual void OnCollision(Collider* pOther) = 0;
	virtual void OnCollisionEnter(Collider* pOther) = 0;
	virtual void OnCollisionExit(Collider* pOther) = 0;

	Collider* CreateCollider();
	RigidBody* CreateRigidBody();
	Gravity* CreateGravity();
	Camera* CreateCamera(const Vector3* pPos, const Vector3* pYawPitchRoll = nullptr);

	class N_Animation* CreateAnimation(AkU32 uMaxClipNum);
	void SetAnimation(ANIM_STATE eState);

	void DestroyCollider();
	void DesteoyRigidBody();
	void DestroyGravity();
	void DestroyCamera();

	void DestroyAnimation();

	Collider* GetCollider() { return _pCollider; }
	RigidBody* GetRigidBody() { return _pRigidBody; }
	Gravity* GetGravity() { return _pGravity; }
	Camera* GetCamera() { return _pCamera; }

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

	// Weapon
	Weapon* _pWeapon = nullptr;

	N_Animation* _pAnimation = nullptr;

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
	MOVE_STATE MoveState = {};
	ANIM_STATE AnimState = ANIM_STATE::PLAYER_ANIM_STATE_COUNT;
};
