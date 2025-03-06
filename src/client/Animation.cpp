#include "pch.h"
#include "Animation.h"
#include "ModelImporter.h"
#include "Application.h"
#include "SceneManager.h"
#include "SceneLoading.h"

BoneAnimation_t::BoneAnimation_t()
{
}

BoneAnimation_t::~BoneAnimation_t()
{
	CleanUp();
}

void BoneAnimation_t::Interpolate(const AkF32 fTimePos, Matrix* pOutBoneTransform)
{
	if (!uNumKeyFrame)
	{
		return;
	}

	AkU32 uFront = 0;
	AkU32 uBack = uNumKeyFrame - 1;

	if (fTimePos <= pKeyFrameList[uFront].fTimePos)
	{
		Vector3 vScale = pKeyFrameList[uFront].vScale;
		Quaternion qRot = pKeyFrameList[uFront].qRot;
		Vector3 vPos = pKeyFrameList[uFront].vPos;

		*pOutBoneTransform = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qRot) * Matrix::CreateTranslation(vPos);
	}
	else if (fTimePos >= pKeyFrameList[uBack].fTimePos)
	{
		Vector3 vScale = pKeyFrameList[uBack].vScale;
		Quaternion qRot = pKeyFrameList[uBack].qRot;
		Vector3 vPos = pKeyFrameList[uBack].vPos;

		*pOutBoneTransform = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qRot) * Matrix::CreateTranslation(vPos);
	}
	else
	{
		for (AkI32 i = 0; i < (AkI32)uNumKeyFrame - 1; i++)
		{
			if (pKeyFrameList[i].fTimePos <= fTimePos && fTimePos <= pKeyFrameList[i + 1].fTimePos)
			{
				const float fLerpRatio = (fTimePos - pKeyFrameList[i].fTimePos) / (pKeyFrameList[i + 1].fTimePos - pKeyFrameList[i].fTimePos);

				Vector3 vScale0 = pKeyFrameList[i].vScale;
				Vector3 vScale1 = pKeyFrameList[i + 1].vScale;

				Vector3 vPos0 = pKeyFrameList[i].vPos;
				Vector3 vPos1 = pKeyFrameList[i + 1].vPos;

				Quaternion qRot0 = pKeyFrameList[i].qRot;
				Quaternion qRot1 = pKeyFrameList[i + 1].qRot;

				Vector3 vScale = DirectX::XMVectorLerp(vScale0, vScale1, fLerpRatio);
				Vector3 vPos = DirectX::XMVectorLerp(vPos0, vPos1, fLerpRatio);
				Quaternion qRot = DirectX::XMQuaternionSlerp(qRot0, qRot1, fLerpRatio);

				*pOutBoneTransform = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qRot) * Matrix::CreateTranslation(vPos);
			}
		}
	}
}

float BoneAnimation_t::GetStartTime()
{
	if (!uNumKeyFrame)
	{
		return 0.0f;
	}

	AkU32 uFront = 0;
	return pKeyFrameList[uFront].fTimePos;
}

float BoneAnimation_t::GetEndTime()
{
	if (!uNumKeyFrame)
	{
		return 0.0f;
	}

	AkU32 uBack = uNumKeyFrame - 1;
	return pKeyFrameList[uBack].fTimePos;
}

void BoneAnimation_t::CleanUp()
{
	if (pKeyFrameList)
	{
		delete[] pKeyFrameList;
		pKeyFrameList = nullptr;
	}
}

AnimationClip_t::AnimationClip_t()
{
}

AnimationClip_t::~AnimationClip_t()
{
	CleanUp();
}

void AnimationClip_t::Interpolate(const AkF32 fTimePos, Matrix* pBoneTransform)
{
	for (AkU32 i = 0; i < uNumBoneAnimation; i++)
	{
		pBoneAnimationList[i].Interpolate(fTimePos, &pBoneTransform[i]);
	}
}

float AnimationClip_t::GetClipStartTime()
{
	float fMin = FLT_MAX;
	for (AkU32 i = 0; i < uNumBoneAnimation; i++)
	{
		fMin = DirectX::XMMin(fMin, pBoneAnimationList[i].GetStartTime());
	}

	return fMin;
}

float AnimationClip_t::GetClipEndTime()
{
	float fMax = 0.0f;
	for (AkU32 i = 0; i < uNumBoneAnimation; i++)
	{
		fMax = DirectX::XMMax(fMax, pBoneAnimationList[i].GetEndTime());
	}

	return fMax;
}

void AnimationClip_t::CleanUp()
{
	if (pBoneAnimationList)
	{
		delete[] pBoneAnimationList;
		pBoneAnimationList = nullptr;
	}
}

/*
==============
Animator
==============
*/

HashTable_t* UAnimation::_pAnimationClipTable;
AkU32 UAnimation::_uInitRefCount;

UAnimation::UAnimation()
{
}

UAnimation::~UAnimation()
{
	CleanUp();
}

AkBool UAnimation::Initialize(AkU32 uMaxClipNum)
{
	// Animation clip 은 공유하는 구조.
	if (!_uInitRefCount)
	{
		_pAnimationClipTable = HT_CreateHashTable(uMaxClipNum, _MAX_PATH, uMaxClipNum);
	}

	_uMaxClipNum = uMaxClipNum;
	_uInitRefCount++;

	return AK_TRUE;
}

void UAnimation::AddRef()
{
	AkU32 uRefCount = ++_uRefCount;
}

void UAnimation::Release()
{
	if (_pFinalTransforms)
	{
		free(_pFinalTransforms);
		_pFinalTransforms = nullptr;
	}
	if (_pRootTransforms)
	{
		free(_pRootTransforms);
		_pRootTransforms = nullptr;
	}

	AkU32 uRefCount = --_uRefCount;

	if (!uRefCount)
	{
		delete this;
	}
}

void UAnimation::GetFinalTransform(const wchar_t* wcClipName, const AkF32 fTimePos, Matrix* pFinalTransform, Matrix* pRootTransform, AkBool bInPlace)
{
	Matrix* pToParentTransform = new Matrix[96];

	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	pAnimationClip->Interpolate(fTimePos, pToParentTransform);

	Matrix* pToRootTransform = new Matrix[_uBoneNum];

	pToRootTransform[0] = pToParentTransform[0];

	if (bInPlace)
	{
		// 제자리 애니메이션 적용.
		Vector3 vTranslation = pToRootTransform[0].Translation();

		// For Jump.
		AkF32 fY = vTranslation.y * 0.85f;

		// Init root transform.
		pToRootTransform[0].Translation(Vector3(0.0f, fY, 0.0f));
	}

	for (AkU32 i = 1; i < _uBoneNum; i++)
	{
		AkI32 iParentIndex = _pBoneHierarchyList[i];

		pToRootTransform[i] = pToParentTransform[i] * pToRootTransform[iParentIndex];
	}

	for (AkU32 i = 0; i < _uBoneNum; i++)
	{
		// WARNING!!
		// default matrix
		pRootTransform[i] = _pBoneOffsetMatrixList[i] * pToRootTransform[i];

		pFinalTransform[i] = _mDefaultMatrix.Invert() * _pBoneOffsetMatrixList[i] * pToRootTransform[i] * _mDefaultMatrix;
		pFinalTransform[i] = pFinalTransform[i].Transpose();
	}

	delete[] pToRootTransform;
	pToRootTransform = nullptr;

	delete[] pToParentTransform;
	pToParentTransform = nullptr;
}

AkU32 UAnimation::GetClipTickPerSecond(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->uTickPerSecond;
}

AkU32 UAnimation::GetClipDuration(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->uDuration;
}

AkF32 UAnimation::GetClipStartTime(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->GetClipStartTime();
}

AkF32 UAnimation::GetClipEndTime(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->GetClipEndTime();
}

AkF32 UAnimation::GetClipCurrentTime(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->GetClipCurrentTime();
}

void UAnimation::SetDefaultMatrix(const Matrix* mDefaultMat)
{
	_mDefaultMatrix = *mDefaultMat;




	// TODO!!

	_pFinalTransforms = reinterpret_cast<Matrix*>(malloc(sizeof(Matrix) * 96));
	memset(_pFinalTransforms, 0, sizeof(Matrix) * 96);

	_pRootTransforms = reinterpret_cast<Matrix*>(malloc(sizeof(Matrix) * 96));
	memset(_pRootTransforms, 0, sizeof(Matrix) * 96);
}

void UAnimation::DestroyAnimationClip(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimClip = nullptr;
	AkU32 uKeyLength = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (HT_Find(_pAnimationClipTable, (void**)&pAnimClip, 1, wcClipName, uKeyLength))
	{
		if (pAnimClip)
		{
			if (pAnimClip->pSearchHandle)
			{
				HT_Delete(_pAnimationClipTable, pAnimClip->pSearchHandle);
				pAnimClip->pSearchHandle = nullptr;
			}

			delete[] pAnimClip;
		}
	}
}

AnimationClip_t* UAnimation::ReadFromAnimationFile(UApplication* pApp, const wchar_t* wcBasePath, const wchar_t* wcFilename)
{
	USceneManager* pSceneManager = pApp->GetSceneManager();
	USceneLoading* pSceneLoading = (USceneLoading*)pSceneManager->GetCurrentScene();

	UModelImporter tModelImporter = { };

	tModelImporter.LoadAnimation(pApp, wcBasePath, wcFilename, _uBoneNum);

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcFilename);
	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	AnimationClip_t* pAnimClip = tModelImporter.GetAnimationClip();

	AddAnimationClip(pAnimClip, wcFilename);

	return pAnimClip;
}

AkBool UAnimation::PlayAnimation(const AkF32 fDeltaTime, const wchar_t* wcAnimClipname, AkBool bInPlace)
{
	AkBool bIsEnd = AK_FALSE;

	AkU32 uTickPerSecond = GetClipTickPerSecond(wcAnimClipname);
	AkU32 uDuration = GetClipDuration(wcAnimClipname);

	AkF32 fAnimTime = GetClipCurrentTime(wcAnimClipname);
	fAnimTime += (AkF32)uTickPerSecond * fDeltaTime;

	if (fAnimTime >= GetClipEndTime(wcAnimClipname))
	{
		fAnimTime = 0.0f;
		bIsEnd = AK_TRUE;
	}

	GetFinalTransform(wcAnimClipname, fAnimTime, _pFinalTransforms, _pRootTransforms, bInPlace);
	SetClipCurrentTime(wcAnimClipname, fAnimTime);

	return bIsEnd;
}

void UAnimation::SetClipCurrentTime(const wchar_t* wcClipName, AkF32 fCurTime)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	pAnimationClip->SetClipCurrentTime(fCurTime);
}

void UAnimation::AddAnimationClip(AnimationClip_t* pAnimClip, const wchar_t* wcClipName)
{
	AkU32 uClipNameByteSize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);

	if (HT_Find(_pAnimationClipTable, (void**)&pAnimClip, 1, wcClipName, uClipNameByteSize))
	{
		return;
	}

	void* pSearchHandle = HT_Insert(_pAnimationClipTable, pAnimClip, wcClipName, uClipNameByteSize);
	pAnimClip->pSearchHandle = pSearchHandle;
}

void UAnimation::CleanUp()
{
	if (_pBoneOffsetMatrixList)
	{
		delete[] _pBoneOffsetMatrixList;
		_pBoneOffsetMatrixList = nullptr;
	}
	if (_pBoneHierarchyList)
	{
		delete[] _pBoneHierarchyList;
		_pBoneHierarchyList = nullptr;
	}
	if (_pAnimationClipTable)
	{
		AkU32 uInitRefCount = --_uInitRefCount;

		if (!uInitRefCount)
		{
			HT_DestroyHashTable(_pAnimationClipTable);
			_pAnimationClipTable = nullptr;
		}
	}
}

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

UAnimator::UAnimator()
{
}

UAnimator::~UAnimator()
{
	CleanUp();
}

AkBool UAnimator::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	return AK_TRUE;
}

void UAnimator::AddAnimation(GAME_ANIMATION_TYPE eType, const wchar_t* wcBasePath, const wchar_t** wcFilenames, AkU32 uFileNum, AnimatorHandle_t* pAnimHandle)
{
	UAnimation* pAnim = AllocAnimation(pAnimHandle);

	AnimationClip_t* pAnimClip = nullptr;

	for (AkU32 i = 0; i < uFileNum; i++)
	{
		pAnimClip = pAnim->ReadFromAnimationFile(_pApp, wcBasePath, wcFilenames[i]);
	}

	_ppAnimList[(AkU32)eType] = pAnim;
}

void UAnimator::CleanUp()
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

UAnimation* UAnimator::AllocAnimation(AnimatorHandle_t* pAnimHandle)
{
	UAnimation* pAnim = new UAnimation;
	pAnim->Initialize(MAX_ANIM_CLIP_COUNT);
	pAnim->SetBoneOffsetMat(pAnimHandle->pBoneOffsetMatList);
	pAnim->SetBoneHierarchy(pAnimHandle->pBoneHierarchyList);
	pAnim->SetBoneNum(pAnimHandle->uBoneNum);
	pAnim->SetDefaultMatrix(&pAnimHandle->mDefaultMat);

	return pAnim;
}

void UAnimator::FreeAnimation(UAnimation* pAnim)
{
	if (pAnim)
	{
		pAnim->Release();
		pAnim = nullptr;
	}
}
