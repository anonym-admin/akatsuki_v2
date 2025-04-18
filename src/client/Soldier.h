#pragma once

#include "Player.h"

class Weapon;

class Soldier : public Player
{
	struct WeaponInfo
	{
		AkI32 iBoneID;
		Vector3 vScale;
		Vector3 vYawPitchRoll;
		Vector3 vPosition;
	};

public:
	enum ANIM_STATE // Scene Loading 의 Animation clip 추가 순서와 동일해야한다.
	{
		IDLE,
		RIFLE_IDLE,
		WALK,
		RIFLE_WALK,
		FIRE_STOP,
		COUNT = AssetAnimationContainer_t::MAX_CLIP_NAME_COUNT,
	} AnimState;

	wchar_t ANIM_CLIP[(AkU32)ANIM_STATE::COUNT][MAX_PATH] = {};

public:
	Soldier();
	Soldier(const wchar_t* wcFile);
	virtual ~Soldier();

	AkBool Initialize();
	AkBool Initialize(const wchar_t* wcFile);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void RenderDepthMap() override;
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
	void ChangeCamera();

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

	Camera* _pCameraAtAimMode = nullptr;
	Camera* _pPendingCam = nullptr;
	
	class Sprite* _pSprite = nullptr;

	// WeaponInfo pWeaponInfoList[COUNT] = {};
	std::unordered_map<std::wstring, std::unordered_map<std::wstring, WeaponInfo>> _mapWeaponInfo = {};

public:
	AkBool Attack = AK_FALSE;
	AkBool Aim = AK_FALSE;
};

void SetIdle(Actor* pSwat);
void SetNextPunching(Actor* pSwat);
void SetNextFire(Actor* pSwat);
