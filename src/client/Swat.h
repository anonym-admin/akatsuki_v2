#pragma once

#include "Player.h"

class Weapon;

class Swat : public Player
{
public:
	enum ANIM_STATE
	{
		IDLE,
		// Walk
		F_WALK,
		FL_WALK,
		FR_WALK,
		L_WALK,
		R_WALK,
		BL_WALK,
		BR_WALK,
		B_WALK,
		// Run
		RUN,

		COUNT = 32,
	} AnimState;

	const wchar_t* ANIM_CLIP[(AkU32)ANIM_STATE::COUNT] =
	{
		 L"SwatGuy_Idle.anim",
		 L"SwatGuy_FrontWalk.anim",
		 L"SwatGuy_FrontLeftWalk.anim",
		 L"SwatGuy_FrontRightWalk.anim",
		 L"SwatGuy_LeftWalk.anim",
		 L"SwatGuy_RightWalk.anim",
		 L"SwatGuy_BackLeftWalk.anim",
		 L"SwatGuy_BackRightWalk.anim",
		 L"SwatGuy_BackWalk.anim",
		 L"SwatGuy_Run.anim",
	};

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

	void SetAnimation(ANIM_STATE eState, AkF32 fSpeed = 1.5f);

private:
	void CleanUp();

	void SetIdle();
	void UpdateMove();
	void UpdateWeapon();
	void UpdateFire();
	void FinalUpdateWeapon();

private:
	Matrix _mHandAnimTransform = Matrix();
};

