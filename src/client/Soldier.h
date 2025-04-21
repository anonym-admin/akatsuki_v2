#pragma once

#include "Player.h"

class Weapon;
class Spark;

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
		WALK_BACK,
		WALK_RIGHT,
		WALK_RIGHT_FDIAG,
		WALK_RIGHT_BDIAG,
		WALK_LEFT,
		WALK_LEFT_FDIAG,
		WALK_LEFT_BDIAG,
		RIFLE_WALK,
		RIFLE_WALK_BACK,
		RIFLE_WALK_RIGHT,
		RIFLE_WALK_RIGHT_FDIAG,
		RIFLE_WALK_RIGHT_BDIAG,
		RIFLE_WALK_LEFT,
		RIFLE_WALK_LEFT_FDIAG,
		RIFLE_WALK_LEFT_BDIAG,
		RIFLE_RUN,
		RIFLE_RUN_BACK,
		RIFLE_RUN_RIGHT,
		RIFLE_RUN_RIGHT_FDIAG,
		RIFLE_RUN_RIGHT_BDIAG,
		RIFLE_RUN_LEFT,
		RIFLE_RUN_LEFT_FDIAG,
		RIFLE_RUN_LEFT_BDIAG,
		FIRE_STOP,
		PUNCH_STOP,
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
	void SetAnimation(ANIM_STATE eState, AkF32 fSpeed = 1.5f);

	AkF32 GetWalkSpeed() { return _fWalkSpeed; }
	AkF32 GetRunSpeed() { return _fRunSpeed; }
	Weapon* GetWeapon() { return _pWeapon; }

	void AddForce(Collider* pOther);
	void ChangeCamera();

private:
	void CleanUp();

	void UpdateMove();
	void UpdateWeapon();
	void FinalUpdateWeapon();

private:
	Matrix _mHandAnimTransform = Matrix();
	AkF32 _fWalkSpeed = 2.65f;
	AkF32 _fRunSpeed = 3.5f;
	AkBool _bDrawSpark = AK_FALSE;
	AkF32 _fAnimSpeed = 1.5f;

	Camera* _pCameraAtAimMode = nullptr;
	Camera* _pPendingCam = nullptr;

	ISprite* _pAimSprite = nullptr;
	AkI32 _iAimRenderPosX = 0;
	AkI32 _iAimRenderPosY = 0;

	Spark* _pSpark = nullptr;

	std::unordered_map<std::wstring, std::unordered_map<std::wstring, WeaponInfo>> _mapWeaponInfo = {};

public:
	AkBool Attack = AK_FALSE;
	AkBool Aim = AK_FALSE;
	AkBool LBtnUp = AK_FALSE;
};

void SetIdle(Actor* pSwat);
void SetNextPunching(Actor* pSwat);
