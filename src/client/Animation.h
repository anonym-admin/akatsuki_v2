#pragma once

class Actor;

struct KeyFrame_t
{
	Vector3 vPos = Vector3(0.0f);
	Vector3 vScale = Vector3(1.0f);
	Quaternion qRot = Quaternion();
	AkF32 fTimePos;
};

struct BoneAnimation_t
{
	BoneAnimation_t();
	~BoneAnimation_t();

	void Interpolate(const AkF32 fTimePos, Matrix* pOutBoneTransform);
	AkF32 GetStartTime();
	AkF32 GetEndTime();
	void CleanUp();

	KeyFrame_t* pKeyFrameList = nullptr;
	AkU32 uNumKeyFrame = 0;
};

struct AnimationClip_t
{
	typedef void(*CALL_BACK)(Actor*);

	AnimationClip_t();
	~AnimationClip_t();

	void Interpolate(const AkF32 fTimePos, Matrix* pBoneTransform);
	AkF32 GetClipStartTime();
	AkF32 GetClipEndTime();
	AkF32 GetClipCurrentTime() { return fCurTime; }
	void SetClipCurrentTime(AkF32 fClipTime) { fCurTime = fClipTime; }
	void CleanUp();

	BoneAnimation_t* pBoneAnimationList = nullptr;
	AkU32 uNumBoneAnimation = 0;
	AkU32 uDuration = 0;
	AkU32 uTickPerSecond = 0;
	AkF32 fCurTime = 0;
	AkU32 uMaxKeyFrame = 0;

	CALL_BACK pCallBack = nullptr;
	Actor* pActor = nullptr;

	// Hash Table 버킷을 지우기 위한 핸들.
	void* pSearchHandle = nullptr;

	List_t tLink = {};
};

/*
============
Animation
============
*/

class Animation
{
	struct Animator_t
	{
		AkF32 fFrameWeight = 0.0f;
		AkU32 uCurFrame = 0;
		AkU32 uNextFrame = 1;
		const wchar_t* wcName = nullptr;
		ANIM_CLIP_STATE eAnimState = ANIM_CLIP_STATE::STOP;
	};

	void UpdateAnimator(Animator_t* pAnimator);

public:
	Animation(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum);
	~Animation();

	AkBool Initialize(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum);
	void Update();

	Matrix GetBoneTrnasformAtID(AkU32 uBoneID);
	Matrix* GetBoneTransforms();

	void SetEndCallBack(const wchar_t* wcClipname, Actor* pActor, AnimationClip_t::CALL_BACK pCallBack);
	AnimationClip_t* ReadClip(const wchar_t* wcBasePath, const wchar_t* wcClipname);
	void PlayClip(const wchar_t* wcClipname, ANIM_CLIP_STATE eState, AkF32 fSpeed = 1.0f, AkF32 fBlendTime = 0.2f);

private:
	void CleanUp();

	void AddAnimationClip(AnimationClip_t* pAnimClip, const wchar_t* wcClipName);
	void SetBoneNum(AkU32 uBoneNum) { _uBoneNum = uBoneNum; }
	void SetDefaultMatrix(const Matrix* pDefaultMat) { _mDefaultMatrix = *pDefaultMat; }
	void SetBoneOffsetMat(const Matrix* pBoneOffsetMatList) { _pBoneOffsetMatrixList = pBoneOffsetMatList; }
	void SetBoneHierarchy(const AkI32* pBoneHierarchyList) { _pBoneHierarchyList = pBoneHierarchyList; }

private:
	HashTable_t* _pAnimationClipTable = nullptr;
	List_t* _pClipHead = nullptr;
	List_t* _pClipTail = nullptr;
	Animator_t _tCurAnimator = {};
	Animator_t _tNextAnimator = {};
	const AkI32* _pBoneHierarchyList = nullptr;
	const Matrix* _pBoneOffsetMatrixList = nullptr;
	Matrix _mDefaultMatrix = Matrix();
	Matrix* _pFinalTransforms = nullptr;
	AkU32 _uBoneNum = 0;
	AkU32 _uMaxClipNum = 0;
	AkBool _bIsChanging = AK_FALSE;
	AkF32 _fChangedTime = 0.0f;
	AkF32 _fBlendTime = 0.0f;
	AkF32 _fAnimScale = 1.0f;
};