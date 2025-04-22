#pragma once

#include "Weapon.h"
#include <queue>

/*
=============
BRS_74
=============
*/

class Sprite;
class Sound;
class Casing;

class BRS_74 : public Weapon
{
public:
	static const AkU32 MAX_CASING_COUNT = 256;

	BRS_74();
	BRS_74(const wchar_t* wcScript);
	~BRS_74();

	AkBool Initialize();
	AkBool Initialize(const wchar_t* wcScript);
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void RenderShadowMaps() override;
	virtual void RenderDepthMap() override;
	virtual void Render() override;

	virtual void OnCollision(Collider* pOther) override;
	virtual void OnCollisionEnter(Collider* pOther) override;
	virtual void OnCollisionExit(Collider* pOther) override;

	virtual BRS_74* Clone() override;

	void Fire();
	void Release();

private:
	void CleanUp();

	Casing* CreateCasing();
	void DestroyCasing(Casing* pCasing);
	void DestroyAllCasing();

private:
	AkU32 _uMaxFireBullet = 5;

	Sprite* _pMuzzleEffect = nullptr;
	Sound* _pFireSound = nullptr;

	AkBool _bFire = AK_FALSE;
	AkBool _bFirst = AK_TRUE;

	// 이후에 Casing 오브젝트로 이동
	Sound* _pCasingBounceSound = nullptr;
	Casing* _pCasings[MAX_CASING_COUNT] = {};
	AkU32 _uCasingCount = 0;
};

