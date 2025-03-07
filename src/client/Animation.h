#pragma once

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

	// Hash Table 버킷을 지우기 위한 핸들.
	void* pSearchHandle = nullptr;
};

/*
==============
Animator
==============
*/

struct HashTable_t;

class Animation
{
public:
	Animation();
	~Animation();

	AkBool Initialize(AkU32 uMaxClipNum);
	void AddRef();
	void Release();

	void GetFinalTransform(const wchar_t* wcClipName, const AkF32 fTimePos, Matrix* pFinalTransform, Matrix* pRootTransform, AkBool bInPlace);
	AkU32 GetClipTickPerSecond(const wchar_t* wcClipName);
	AkU32 GetClipDuration(const wchar_t* wcClipName);
	AkF32 GetClipStartTime(const wchar_t* wcClipName);
	AkF32 GetClipEndTime(const wchar_t* wcClipName);
	AkF32 GetClipCurrentTime(const wchar_t* wcClipName);
	AkU32 GetBoneNum() { return _uBoneNum; }
	Matrix* GetFinalTransforms() { return _pFinalTransforms; }
	Matrix* GetRootTransforms() { return _pRootTransforms; }

	void SetDefaultMatrix(const Matrix* mDefaultMat);
	void SetBoneOffsetMat(const Matrix* pBoneOffsetMatList) { _pBoneOffsetMatrixList = pBoneOffsetMatList; }
	void SetBoneHierarchy(const AkI32* pBoneHierarchyList) { _pBoneHierarchyList = pBoneHierarchyList; }
	void SetClipCurrentTime(const wchar_t* wcClipName, AkF32 fCurTime);
	void SetBoneNum(AkU32 uBoneNum) { _uBoneNum = uBoneNum; }
	void DestroyAnimationClip(const wchar_t* wcClipName);

	AnimationClip_t* ReadFromAnimationFile(const wchar_t* wcBasePath, const wchar_t* wcFilename);

	AkBool PlayAnimation(const wchar_t* wcAnimClipname, AkBool bInPlace);

private:
	void CleanUp();

	void AddAnimationClip(AnimationClip_t* pAnimClip, const wchar_t* wcClipName);

private:
	// 이후에 static 으로 관리?? 
	// TODO!!
	static HashTable_t* _pAnimationClipTable;
	static AkU32 _uInitRefCount;

	const AkI32* _pBoneHierarchyList = nullptr;
	const Matrix* _pBoneOffsetMatrixList = nullptr;
	AkU32 _uBoneNum = 0;
	AkU32 _uMaxClipNum = 0;
	Matrix _mDefaultMatrix = Matrix();
	AkU32 _uRefCount = 1;

	Matrix* _pFinalTransforms = nullptr;
	Matrix* _pRootTransforms = nullptr;
};









// Newly
enum class N_ANIM_STATE
{
	LOOP,
	ONCE,
	STOP,
};

class N_Animation
{
	struct N_Animator
	{
		AkF32 fFrameWeight = 0.0f;
		AkU32 uCurFrame = 0;
		AkU32 uNextFrame = 1;
		const wchar_t* wcName = nullptr;
		N_ANIM_STATE eAnimState = N_ANIM_STATE::STOP;
	};

	void UpdateAnimator(N_Animator* pAnimator);

public:
	N_Animation(AkU32 uMaxClipNum);
	~N_Animation();

	AkBool Initialize(AkU32 uMaxClipNum);
	void Update();

	Matrix GetBoneTrnasformAtID(AkU32 uBoneID);
	Matrix* GetBoneTransforms();
	
	AnimationClip_t* ReadClip(const wchar_t* wcBasePath, const wchar_t* wcClipname);
	void SetIdle(const wchar_t* wcIdleClipName);
	void PlayClip(const wchar_t* wcClipname, N_ANIM_STATE eState, AkF32 fBlendTime);
	void SetBoneNum(AkU32 uBoneNum) { _uBoneNum = uBoneNum; }
	void SetDefaultMatrix(const Matrix* pDefaultMat) { _mDefaultMatrix = *pDefaultMat; }
	void SetBoneOffsetMat(const Matrix* pBoneOffsetMatList) { _pBoneOffsetMatrixList = pBoneOffsetMatList; }
	void SetBoneHierarchy(const AkI32* pBoneHierarchyList) { _pBoneHierarchyList = pBoneHierarchyList; }

private:
	void CleanUp();

	void AddAnimationClip(AnimationClip_t* pAnimClip, const wchar_t* wcClipName);

private:
	HashTable_t* _pAnimationClipTable = nullptr;
	N_Animator _tCurAnimator = {};
	N_Animator _tNextAnimator = {};
	const AkI32* _pBoneHierarchyList = nullptr;
	const Matrix* _pBoneOffsetMatrixList = nullptr;
	Matrix _mDefaultMatrix = Matrix();
	Matrix* _pFinalTransforms = nullptr;
	AkU32 _uBoneNum = 0;
	AkU32 _uMaxClipNum = 0;
	AkBool _bIsChanging = AK_FALSE;
	AkF32 _fChangedTime = 0.0f;
	AkF32 _fBlendTime = 0.0f;
	AkF32 _fAnimScale = 1.5f;
};