#pragma once

#include "Player.h"

class Weapon;

class Swat : public Player
{
public:
	enum ANIM_STATE
	{
		/*Idle*/
		IDLE,

		/*Walk*/
		F_WALK,
		FL_WALK,
		FR_WALK,
		L_WALK,
		R_WALK,
		BL_WALK,
		BR_WALK,
		B_WALK,

		/*Run*/
		F_RUN,
		FL_RUN,
		FR_RUN,

		/*Attack*/
		PUNCHING_01,
		PUNCHING_02,

		/*Jump*/
		RUN_JUMP,
		IDLE_JUMP,

		COUNT = AssetAnimationContainer_t::MAX_CLIP_NAME_COUNT,

	} AnimState;

	const wchar_t* ANIM_CLIP[(AkU32)ANIM_STATE::COUNT] = {};
	//{
	//	 L"SwatGuy_Idle.anim",
	//	 L"SwatGuy_FrontWalk.anim",
	//	 L"SwatGuy_FrontLeftWalk.anim",
	//	 L"SwatGuy_FrontRightWalk.anim",
	//	 L"SwatGuy_LeftWalk.anim",
	//	 L"SwatGuy_RightWalk.anim",
	//	 L"SwatGuy_BackLeftWalk.anim",
	//	 L"SwatGuy_BackRightWalk.anim",
	//	 L"SwatGuy_BackWalk.anim",
	//	 L"SwatGuy_Run.anim",
	//	 L"SwatGuy_FrontLeftRun.anim",
	//	 L"SwatGuy_FrontRightRun.anim",
	//	 L"SwatGuy_Punching_01.anim",
	//	 L"SwatGuy_Punching_02.anim",
	//	 L"SwatGuy_RunJump.anim",
	//	 L"SwatGuy_IdleJump.anim"
	//};

public:
	Swat();
	virtual ~Swat();

	AkBool Initialize();
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	void SetIdle();
	void SetNextPunching();
	void SetAnimation(ANIM_STATE eState, AkF32 fSpeed = 1.5f);

	AkF32 GetRunSpeed() { return _fRunSpeed; }
	AkF32 GetWalkSpeed() { return _fWalkSpeed; }

private:
	void CleanUp();

	void UpdateMove();
	void UpdateWeapon();
	void UpdateFire();
	void FinalUpdateWeapon();

private:
	Matrix _mHandAnimTransform = Matrix();
	AkF32 _fWalkSpeed = 2.65f;
	AkF32 _fRunSpeed = 3.5f;

public:
	AkBool Attack = AK_FALSE;
};

void SetIdle(Actor* pSwat);
void SetNextPunching(Actor* pSwat);

