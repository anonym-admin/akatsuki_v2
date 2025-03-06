#include "pch.h"
#include "Animator.h"
#include "Animation.h"

/*
===============================
Animation file path
===============================
*/

const wchar_t* GMAE_ANIM_FILE_BASE_PATH = L"../../assets/model/";

const wchar_t* GAME_ANIM_PLAYER_ANIM_FILE_NAME[] =
{
		L"SwatGuy_Idle.anim"
	,	L"SwatGuy_Walking.anim"
	,   L"SwatGuy_Run.anim"
	,   L"SwatGuy_Jump.anim"
	,   L"SwatGuy_RifleRun.anim"
	,	L"SwatGuy_RifleWalking.anim"
	,	L"SwatGuy_RifleIdle.anim"
	,	L"SwatGuy_RifleRunFire.anim"
	,	L"SwatGuy_RifleWalkFire.anim"
	,	L"SwatGuy_RifleIdleFire.anim"
};

const wchar_t* GAME_ANIM_DANCER_ANIM_FILE_NAME[] =
{
		L"Dancing.anim"
	,
};


/*
=============
Animator
=============
*/

Animator::~Animator()
{
	CleanUp();
}

void Animator::AddAnimation(GAME_ANIMATION_TYPE eType, const wchar_t* wcBasePath, const wchar_t** wcFilenames, AkU32 uFileNum, AnimatorHandle_t* pAnimHandle)
{
	Animation* pAnim = AllocAnimation(pAnimHandle);

	AnimationClip_t* pAnimClip = nullptr;

	for (AkU32 i = 0; i < uFileNum; i++)
	{
		pAnimClip = pAnim->ReadFromAnimationFile(_pApp, wcBasePath, wcFilenames[i]);
	}

	_ppAnimList[(AkU32)eType] = pAnim;
}

void Animator::CleanUp()
{
	if (_ppAnimList[(AkU32)GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_PLAYER])
	{
		for (AkU32 i = 0; i < _countof(GAME_ANIM_PLAYER_ANIM_FILE_NAME); i++)
		{
			_ppAnimList[(AkU32)GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_PLAYER]->DestroyAnimationClip(GAME_ANIM_PLAYER_ANIM_FILE_NAME[i]);
		}
	}

	if (_ppAnimList[(AkU32)GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_DANCER])
	{
		for (AkU32 i = 0; i < _countof(GAME_ANIM_DANCER_ANIM_FILE_NAME); i++)
		{
			_ppAnimList[(AkU32)GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_PLAYER]->DestroyAnimationClip(GAME_ANIM_DANCER_ANIM_FILE_NAME[i]);
		}
	}

	for (AkU32 i = 0; i < (AkU32)GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_COUNT; i++)
	{
		if (_ppAnimList[i])
		{
			FreeAnimation(_ppAnimList[i]);
			_ppAnimList[i] = nullptr;
		}
	}
}

Animation* Animator::AllocAnimation(AnimatorHandle_t* pAnimHandle)
{
	Animation* pAnim = new Animation;
	pAnim->Initialize(MAX_ANIM_CLIP_COUNT);
	pAnim->SetBoneOffsetMat(pAnimHandle->pBoneOffsetMatList);
	pAnim->SetBoneHierarchy(pAnimHandle->pBoneHierarchyList);
	pAnim->SetBoneNum(pAnimHandle->uBoneNum);
	pAnim->SetDefaultMatrix(&pAnimHandle->mDefaultMat);

	return pAnim;
}

void Animator::FreeAnimation(Animation* pAnim)
{
	if (pAnim)
	{
		pAnim->Release();
		pAnim = nullptr;
	}
}
