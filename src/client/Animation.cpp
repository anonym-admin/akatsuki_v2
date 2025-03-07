#include "pch.h"
#include "Animation.h"
#include "ModelImporter.h"
#include "Application.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "Timer.h"

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

HashTable_t* Animation::_pAnimationClipTable;
AkU32 Animation::_uInitRefCount;

Animation::Animation()
{
}

Animation::~Animation()
{
	CleanUp();
}

AkBool Animation::Initialize(AkU32 uMaxClipNum)
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

void Animation::AddRef()
{
	AkU32 uRefCount = ++_uRefCount;
}

void Animation::Release()
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

void Animation::GetFinalTransform(const wchar_t* wcClipName, const AkF32 fTimePos, Matrix* pFinalTransform, Matrix* pRootTransform, AkBool bInPlace)
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

AkU32 Animation::GetClipTickPerSecond(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->uTickPerSecond;
}

AkU32 Animation::GetClipDuration(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->uDuration;
}

AkF32 Animation::GetClipStartTime(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->GetClipStartTime();
}

AkF32 Animation::GetClipEndTime(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->GetClipEndTime();
}

AkF32 Animation::GetClipCurrentTime(const wchar_t* wcClipName)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	return pAnimationClip->GetClipCurrentTime();
}

void Animation::SetDefaultMatrix(const Matrix* mDefaultMat)
{
	_mDefaultMatrix = *mDefaultMat;




	// TODO!!

	_pFinalTransforms = reinterpret_cast<Matrix*>(malloc(sizeof(Matrix) * 96));
	memset(_pFinalTransforms, 0, sizeof(Matrix) * 96);

	_pRootTransforms = reinterpret_cast<Matrix*>(malloc(sizeof(Matrix) * 96));
	memset(_pRootTransforms, 0, sizeof(Matrix) * 96);
}

void Animation::DestroyAnimationClip(const wchar_t* wcClipName)
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

AnimationClip_t* Animation::ReadFromAnimationFile(const wchar_t* wcBasePath, const wchar_t* wcFilename)
{
	SceneLoading* pSceneLoading = (SceneLoading*)GSceneManager->GetCurrentScene();

	UModelImporter tModelImporter = { };

	tModelImporter.LoadAnimation(wcBasePath, wcFilename, _uBoneNum);

	wchar_t wcFullPath[MAX_PATH] = {};
	wcscpy_s(wcFullPath, wcBasePath);
	wcscat_s(wcFullPath, wcFilename);
	wcscat_s(wcFullPath, L"\n");
	pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	AnimationClip_t* pAnimClip = tModelImporter.GetAnimationClip();

	AddAnimationClip(pAnimClip, wcFilename);

	return pAnimClip;
}

AkBool Animation::PlayAnimation(const wchar_t* wcAnimClipname, AkBool bInPlace)
{
	AkBool bIsEnd = AK_FALSE;

	AkU32 uTickPerSecond = GetClipTickPerSecond(wcAnimClipname);
	AkU32 uDuration = GetClipDuration(wcAnimClipname);

	AkF32 fAnimTime = GetClipCurrentTime(wcAnimClipname);
	fAnimTime += (AkF32)uTickPerSecond * DT;

	if (fAnimTime >= GetClipEndTime(wcAnimClipname))
	{
		fAnimTime = 0.0f;
		bIsEnd = AK_TRUE;
	}

	GetFinalTransform(wcAnimClipname, fAnimTime, _pFinalTransforms, _pRootTransforms, bInPlace);
	SetClipCurrentTime(wcAnimClipname, fAnimTime);

	return bIsEnd;
}

void Animation::SetClipCurrentTime(const wchar_t* wcClipName, AkF32 fCurTime)
{
	AnimationClip_t* pAnimationClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pAnimationClip, 1, wcClipName, uKeySize))
	{
		__debugbreak();
	}

	pAnimationClip->SetClipCurrentTime(fCurTime);
}

void Animation::AddAnimationClip(AnimationClip_t* pAnimClip, const wchar_t* wcClipName)
{
	AkU32 uClipNameByteSize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);

	if (HT_Find(_pAnimationClipTable, (void**)&pAnimClip, 1, wcClipName, uClipNameByteSize))
	{
		return;
	}

	void* pSearchHandle = HT_Insert(_pAnimationClipTable, pAnimClip, wcClipName, uClipNameByteSize);
	pAnimClip->pSearchHandle = pSearchHandle;
}

void Animation::CleanUp()
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



// Newly => Animation Blending.
N_Animation::N_Animation(AkU32 uMaxClipNum)
{
	if (!Initialize(uMaxClipNum))
	{
		__debugbreak();
	}
}

N_Animation::~N_Animation()
{
	CleanUp();
}

AkBool N_Animation::Initialize(AkU32 uMaxClipNum)
{
	_pAnimationClipTable = HT_CreateHashTable(uMaxClipNum, _MAX_PATH, uMaxClipNum);
	_uMaxClipNum = uMaxClipNum;
	_bIsChanging = AK_FALSE;

	_pFinalTransforms = reinterpret_cast<Matrix*>(malloc(sizeof(Matrix) * 96));
	memset(_pFinalTransforms, 0, sizeof(Matrix) * 96);

	return AK_TRUE;
}

void N_Animation::Update()
{
	if (_bIsChanging)
	{
		UpdateAnimator(&_tNextAnimator);

		_fChangedTime += DT;
		if (_fChangedTime > _fBlendTime)
		{
			_fChangedTime = 0.0f;

			_tCurAnimator = _tNextAnimator;
			_bIsChanging = AK_FALSE;
		}
	}

	UpdateAnimator(&_tCurAnimator);
}

Matrix N_Animation::GetBoneTrnasformAtID(AkU32 uBoneID)
{
	return Matrix();
}

Matrix* N_Animation::GetBoneTransforms()
{
	Matrix* pToParentTransform = new Matrix[96];

	AnimationClip_t* pCurClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(_tCurAnimator.wcName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pCurClip, 1, _tCurAnimator.wcName, uKeySize))
	{
		__debugbreak();
	}

	if (!_tNextAnimator.wcName)
		_tNextAnimator.wcName = _tCurAnimator.wcName;

	AnimationClip_t* pNextClip = nullptr;
	uKeySize = (AkU32)wcslen(_tNextAnimator.wcName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pNextClip, 1, _tNextAnimator.wcName, uKeySize))
	{
		__debugbreak();
	}

	if (pNextClip->uNumBoneAnimation != pCurClip->uNumBoneAnimation)
	{
		__debugbreak();
	}

	Matrix* pCurTransform = new Matrix[96];
	Matrix* pNextTransform = new Matrix[96];
	if (_bIsChanging)
	{
		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			Vector3 vCurScale = Vector3(1.0f);
			Quaternion qCurQuat = Quaternion();
			Vector3 vCurPos = Vector3(0.0f);

			Matrix Test = Matrix::CreateScale(vCurScale) * Matrix::CreateFromQuaternion(qCurQuat) * Matrix::CreateTranslation(vCurPos);

			Vector3 vNextScale = Vector3(1.0f);
			Quaternion qNextQuat = Quaternion();
			Vector3 vNextPos = Vector3(0.0f);

			// Cur
			{
				AkU32 uKeyFrame = pCurClip->pBoneAnimationList[i].uNumKeyFrame;
				if (uKeyFrame)
				{
					uKeyFrame = _tCurAnimator.uNextFrame % uKeyFrame;

					Vector3 vScale = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vScale;
					Quaternion qQuat = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].qRot;
					Vector3 vPos = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vPos;

					pCurTransform[i] = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qQuat) * Matrix::CreateTranslation(vPos);

					//vCurScale = vScale;
					//qCurQuat = qQuat;
					//vCurPos = vPos;
				}
			}
			// Next
			{
				AkU32 uKeyFrame = pNextClip->pBoneAnimationList[i].uNumKeyFrame;
				if (uKeyFrame)
				{
					uKeyFrame = _tNextAnimator.uCurFrame % uKeyFrame;

					Vector3 vScale0 = pNextClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vScale;
					Quaternion qQuat0 = pNextClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].qRot;
					Vector3 vPos0 = pNextClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vPos;

					uKeyFrame = pNextClip->pBoneAnimationList[i].uNumKeyFrame;
					uKeyFrame = _tNextAnimator.uNextFrame % uKeyFrame;

					Vector3 vScale1 = pNextClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vScale;
					Quaternion qQuat1 = pNextClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].qRot;
					Vector3 vPos1 = pNextClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vPos;

					Vector3 vScale = DirectX::XMVectorLerp(vScale0, vScale1, _tNextAnimator.fFrameWeight);
					Quaternion qQuat = DirectX::XMQuaternionSlerp(qQuat0, qQuat1, _tNextAnimator.fFrameWeight);
					Vector3 vPos = DirectX::XMVectorLerp(vPos0, vPos1, _tNextAnimator.fFrameWeight);

					pNextTransform[i] = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qQuat) * Matrix::CreateTranslation(vPos);

					//vNextScale = vScale;
					//qNextQuat = qQuat;
					//vNextPos = vPos;
				}
			}

			const AkF32 fRatio = _fChangedTime / _fBlendTime;

			pToParentTransform[i] = pCurTransform[i] * (1.0f - fRatio) + pNextTransform[i] * fRatio;

			//Vector3 vScale = DirectX::XMVectorLerp(vCurScale, vNextScale, fRatio);
			//Quaternion qQuat = DirectX::XMQuaternionSlerp(qCurQuat, qNextQuat, fRatio);
			//Vector3 vPos = DirectX::XMVectorLerp(vCurPos, vNextPos, fRatio);

			//pToParentTransform[i] = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qQuat) * Matrix::CreateTranslation(vPos);
		}
	}
	else
	{
		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			AkU32 uKeyFrame = pCurClip->pBoneAnimationList[i].uNumKeyFrame;
			if (uKeyFrame)
			{
				uKeyFrame = _tCurAnimator.uCurFrame % uKeyFrame;

				Vector3 vScale0 = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vScale;
				Quaternion qQuat0 = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].qRot;
				Vector3 vPos0 = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vPos;

				uKeyFrame = pCurClip->pBoneAnimationList[i].uNumKeyFrame;
				uKeyFrame = _tCurAnimator.uNextFrame % uKeyFrame;

				Vector3 vScale1 = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vScale;
				Quaternion qQuat1 = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].qRot;
				Vector3 vPos1 = pCurClip->pBoneAnimationList[i].pKeyFrameList[uKeyFrame].vPos;

				Vector3 vScale = DirectX::XMVectorLerp(vScale0, vScale1, _tCurAnimator.fFrameWeight);
				Quaternion qQuat = DirectX::XMQuaternionSlerp(qQuat0, qQuat1, _tCurAnimator.fFrameWeight);
				Vector3 vPos = DirectX::XMVectorLerp(vPos0, vPos1, _tCurAnimator.fFrameWeight);

				pToParentTransform[i] = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qQuat) * Matrix::CreateTranslation(vPos);
			}
			else
			{
				pToParentTransform[i] = Matrix();
			}
		}
	}

	delete[] pCurTransform;
	pCurTransform = nullptr;

	delete[] pNextTransform;
	pNextTransform = nullptr;

	Matrix* pToRootTransform = new Matrix[_uBoneNum];

	pToRootTransform[0] = pToParentTransform[0];

	for (AkU32 i = 1; i < _uBoneNum; i++)
	{
		AkI32 iParentIndex = _pBoneHierarchyList[i];

		pToRootTransform[i] = pToParentTransform[i] * pToRootTransform[iParentIndex];
	}

	for (AkU32 i = 0; i < _uBoneNum; i++)
	{
		_pFinalTransforms[i] = _mDefaultMatrix.Invert() * _pBoneOffsetMatrixList[i] * pToRootTransform[i] * _mDefaultMatrix;
		_pFinalTransforms[i] = _pFinalTransforms[i].Transpose();
	}

	delete[] pToRootTransform;
	pToRootTransform = nullptr;

	delete[] pToParentTransform;
	pToParentTransform = nullptr;

	return _pFinalTransforms;
}

AnimationClip_t* N_Animation::ReadClip(const wchar_t* wcBasePath, const wchar_t* wcFilename)
{
	//SceneLoading* pSceneLoading = (SceneLoading*)GSceneManager->GetCurrentScene();

	UModelImporter tModelImporter = { };

	tModelImporter.LoadAnimation(wcBasePath, wcFilename, _uBoneNum);

	//wchar_t wcFullPath[MAX_PATH] = {};
	//wcscpy_s(wcFullPath, wcBasePath);
	//wcscat_s(wcFullPath, wcFilename);
	//wcscat_s(wcFullPath, L"\n");

	//pSceneLoading->RenderLoadingScreenCallBack(wcFullPath);

	AnimationClip_t* pAnimClip = tModelImporter.GetAnimationClip();

	AddAnimationClip(pAnimClip, wcFilename);

	return pAnimClip;
}

void N_Animation::SetIdle(const wchar_t* wcIdleClipName)
{
	_tCurAnimator.wcName = wcIdleClipName;
}

void N_Animation::PlayClip(const wchar_t* wcClipname, N_ANIM_STATE eState, AkF32 fBlendTime)
{
	_fChangedTime = 0.0f;

	_bIsChanging = AK_TRUE;

	_tCurAnimator.eAnimState = N_ANIM_STATE::STOP;
	_tNextAnimator.eAnimState = eState;
	_tNextAnimator.uCurFrame = 0;
	_tNextAnimator.uNextFrame = 1;
	_tNextAnimator.wcName = wcClipname;

	_fBlendTime = fBlendTime;
}

void N_Animation::AddAnimationClip(AnimationClip_t* pAnimClip, const wchar_t* wcClipName)
{
	AkU32 uClipNameByteSize = (AkU32)wcslen(wcClipName) * sizeof(wchar_t);

	if (HT_Find(_pAnimationClipTable, (void**)&pAnimClip, 1, wcClipName, uClipNameByteSize))
	{
		return;
	}

	void* pSearchHandle = HT_Insert(_pAnimationClipTable, pAnimClip, wcClipName, uClipNameByteSize);
	pAnimClip->pSearchHandle = pSearchHandle;
}

void N_Animation::UpdateAnimator(N_Animator* pAnimator)
{
	if (N_ANIM_STATE::LOOP == pAnimator->eAnimState)
	{
		AnimationClip_t* pClip = nullptr;
		AkU32 uKeySize = (AkU32)wcslen(pAnimator->wcName) * sizeof(wchar_t);
		if (!HT_Find(_pAnimationClipTable, (void**)&pClip, 1, pAnimator->wcName, uKeySize))
		{
			__debugbreak();
		}

		pAnimator->fFrameWeight += (DT * pClip->uTickPerSecond * _fAnimScale);

		if (pAnimator->fFrameWeight >= 1.0f) // 프레임 사이를 보간
		{
			pAnimator->fFrameWeight = 0.0f;
			pAnimator->uCurFrame++;
			pAnimator->uNextFrame++;

			// printf("%u %u\n", pAnimator->uCurFrame, pAnimator->uNextFrame);

			// if(pAnimator->uNextFrame >= pClip->u) 
		}
	}
}

void N_Animation::CleanUp()
{
}