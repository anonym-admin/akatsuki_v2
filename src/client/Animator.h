#pragma once

/*
=====================
Game animator type
=====================
*/

enum class GAME_ANIMATION_TYPE
{
	GAME_ANIM_TYPE_PLAYER,
	GAME_ANIM_TYPE_DANCER,
	GAME_ANIM_TYPE_COUNT,
};

extern const wchar_t* GMAE_ANIM_FILE_BASE_PATH;
extern const wchar_t* GAME_ANIM_PLAYER_ANIM_FILE_NAME[10];
extern const wchar_t* GAME_ANIM_DANCER_ANIM_FILE_NAME[1];

struct AnimatorHandle_t
{
	Matrix mDefaultMat = Matrix();
	const Matrix* pBoneOffsetMatList = nullptr;
	const AkI32* pBoneHierarchyList = nullptr;
	AkU32 uBoneNum = 0;
};

/*
=============
Animator
=============
*/

class Animation;

class Animator
{
public:
	static const AkU32 MAX_ANIM_CLIP_COUNT = 16;

	~Animator();

	void AddAnimation(GAME_ANIMATION_TYPE eType, const wchar_t* wcBasePath, const wchar_t** wcFilenames, AkU32 uFileNum, AnimatorHandle_t* pAnimHandle);
	Animation* GetAnimation(GAME_ANIMATION_TYPE eType) { return _ppAnimList[(AkU32)eType]; }

private:
	void CleanUp();

	Animation* AllocAnimation(AnimatorHandle_t* pAnimHandle);
	void FreeAnimation(Animation* pAnim);

private:
	Application* _pApp = nullptr;
	Animation* _ppAnimList[(AkU32)GAME_ANIMATION_TYPE::GAME_ANIM_TYPE_COUNT] = {};
};