#pragma once

#include "Actor.h"

enum class PLAYER_ANIM_STATE
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
	PLAYER_ANIM_STATE_COUNT,
};

enum class PLAYER_MOVE_STATE
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
==========
Player
==========
*/

class UWeapon;

class UPlayer : public UActor
{
public:
	UPlayer();
	~UPlayer();

	virtual AkBool Initialize(UApplication* pApp);
	virtual void Update(const AkF32 fDeltaTime);
	virtual void FinalUpdate(const AkF32 fDeltaTime);
	virtual void RenderShadow();
	virtual void Render();

	virtual void OnCollision(UCollider* pOther);
	virtual void OnCollisionEnter(UCollider* pOther);
	virtual void OnCollisionExit(UCollider* pOther);

	void BindWeapon(UWeapon* pWeapon);

private:
	virtual void CleanUp();

	AkBool _bJumping = AK_FALSE;
	AkBool _bBindWeapon = AK_FALSE;
	AkBool _bFirst = AK_TRUE;
	AkBool _bRightHand = AK_FALSE;
	AkBool _bLeftHand = AK_FALSE;
	AkBool _bFire = AK_FALSE;

	Vector3 _vCollisionPos = Vector3(0.0f);

	UWeapon* _pWeapon = nullptr;
	Matrix _mHandAnimTransform = Matrix();

	PLAYER_MOVE_STATE _eMoveState = {};
	PLAYER_ANIM_STATE _eAnimState = {};
};
