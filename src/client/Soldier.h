#pragma once

#include "Player.h"

class Weapon;

class Soldier : public Player
{
public:
	enum ANIM_STATE
	{
		IDLE,
		WALK,
		COUNT = AssetAnimationContainer_t::MAX_CLIP_NAME_COUNT,

	} AnimState;

	const wchar_t* ANIM_CLIP[(AkU32)ANIM_STATE::COUNT] = {};

public:
	Soldier();
	Soldier(const wchar_t* wcFile);
	virtual ~Soldier();

	AkBool Initialize();
	AkBool Initialize(const wchar_t* wcFile);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void RenderShadowMaps() override;
	virtual void Render() override;

	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	void SetIdle();
	void SetNextPunching();
	void SetNextFire();
	void SetAnimation(ANIM_STATE eState, AkF32 fSpeed = 1.5f);

	AkF32 GetWalkSpeed() { return _fWalkSpeed; }
	AkF32 GetRunSpeed() { return _fRunSpeed; }

	Weapon* GetWeapon() { return _pWeapon; }

	void ActionReaction(Collider* pOther);

private:
	void CleanUp();

	void UpdateMove();
	void UpdateWeapon();
	void UpdateFire();
	void FinalUpdateWeapon();

	void SetWeaponRelativePosition();

private:
	Matrix _mHandAnimTransform = Matrix();
	AkF32 _fWalkSpeed = 2.65f;
	AkF32 _fRunSpeed = 3.5f;
	
	class Sprite* _pSprite = nullptr;

public:
	AkBool Attack = AK_FALSE;
};

void SetIdle(Actor* pSwat);
void SetNextPunching(Actor* pSwat);
void SetNextFire(Actor* pSwat);
