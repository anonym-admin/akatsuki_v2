#pragma once

#include "Weapon.h"

/*
=============
BRS_74
=============
*/

class Sprite;
class Sound;


class BRS_74 : public Weapon
{
public:
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

private:
	AkU32 _uMaxFireBullet = 5;

	Sprite* _pMuzzleEffect = nullptr;
	Sound* _pFireSound = nullptr;

	AkBool _bFire = AK_FALSE;
	AkBool _bFirst = AK_TRUE;
};

