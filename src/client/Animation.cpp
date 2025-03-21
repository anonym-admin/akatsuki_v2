#include "pch.h"
#include "Animation.h"
#include "ModelImporter.h"
#include "Application.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "Timer.h"

static bool useSnap(false);
static float snap[3] = { 1.f, 1.f, 1.f };
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

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

void BoneAnimation_t::Update()
{

}

void BoneAnimation_t::Render()
{
	GRenderer->RenderLineObject(pLineObj, &mWorldRow);
}

void BoneAnimation_t::RenderGUI()
{
	if (!bPick)
	{
		return;
	}

	Vector3 vStartAfterTransform = Vector3(0.0f);
	vStartAfterTransform = Vector3::Transform(vStart, mBoneTransform);

	Matrix mGizmoTransform = Matrix::CreateTranslation(vStartAfterTransform);

	// 첫번째 프레임을 기준으로 수정을 완료하면 나머지 키 프레임에 대해서도 동일한 변환이 적용된다.
	std::string BoneName = pName;
	char Title[_MAX_PATH] = {};
	strcpy_s(Title, std::string(std::string(pName) + " gizmo").c_str());

	ImGuizmo::BeginFrame();
	ImGui::Begin(Title);
	ImGui::Checkbox("Use Gizmo", &bUseGizmo);

	if (!bUseGizmo)
	{
		ImGui::End();
		return;
	}

	if (ImGuizmo::IsUsing())
	{
		ImGui::Text("Using gizmo");
	}
	else
	{
		ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
		ImGui::SameLine();
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
	}

	if (ImGui::IsKeyPressed(ImGuiKey_T))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_E))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) // r Key
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents((float*)&mGizmoTransform._11, matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation);
	ImGui::InputFloat3("Rt", matrixRotation);
	ImGui::InputFloat3("Sc", matrixScale);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&mGizmoTransform._11);

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Y))
		useSnap = !useSnap;
	ImGui::Checkbox(" ", &useSnap);
	ImGui::SameLine();
	switch (mCurrentGizmoOperation)
	{
	case ImGuizmo::TRANSLATE:
		ImGui::InputFloat3("Snap", &snap[0]);
		break;
	case ImGuizmo::ROTATE:
		ImGui::InputFloat("Angle Snap", &snap[0]);
		break;
	case ImGuizmo::SCALE:
		ImGui::InputFloat("Scale Snap", &snap[0]);
		break;
	}

	float windowWidth = (float)ImGui::GetWindowWidth();
	float windowHeight = (float)ImGui::GetWindowHeight();

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	Matrix mView = Matrix();
	Matrix mProj = Matrix();
	GRenderer->GetViewPorjMatrix(&mView, &mProj);
	mView = mView.Transpose();
	mProj = mProj.Transpose();

	ImGuizmo::Manipulate((float*)&mView, (float*)&mProj, mCurrentGizmoOperation, mCurrentGizmoMode, (float*)&mGizmoTransform._11, NULL, useSnap ? &snap[0] : NULL);

	Vector3 vScale = Vector3(1.0f);
	Quaternion qRotation = Quaternion();
	Vector3 vPosition = Vector3(0.0f);
	mGizmoTransform.Decompose(vScale, qRotation, vPosition);

	vPosition -= vStartAfterTransform;
	 
	for (AkU32 i = 0; i < uNumKeyFrame; i++)
	{
		pKeyFrameList[i].vScale = vScale;
		pKeyFrameList[i].qRot = pKeyFrameList[i].qRot * qRotation;
		pKeyFrameList[i].vPos = pKeyFrameList[i].vPos + vPosition;
	}

	ImGui::End();
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
============
Animation
============
*/

Animation::Animation(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum)
{
	if (!Initialize(pMeshDataContainer, wcIdleClipName, uMaxClipNum))
	{
		__debugbreak();
	}
}

Animation::~Animation()
{
	CleanUp();
}

AkBool Animation::Initialize(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum)
{
	// Create HashTable.
	_pAnimationClipTable = HT_CreateHashTable(uMaxClipNum, _MAX_PATH, uMaxClipNum);
	_uMaxClipNum = uMaxClipNum;

	// Create Bone Transform.
	_pFinalTransforms = reinterpret_cast<Matrix*>(malloc(sizeof(Matrix) * 96));
	memset(_pFinalTransforms, 0, sizeof(Matrix) * 96);

	// Set Bone Resource.
	SetBoneHierarchy(pMeshDataContainer->pBoneHierarchyList);
	SetBoneOffsetMat(pMeshDataContainer->pBoneOffsetMatrixList);
	SetDefaultMatrix(&pMeshDataContainer->mDefaultMat);
	SetBoneNum(pMeshDataContainer->uBoneNum);
	SetBoneName(pMeshDataContainer->ppBoneName);

	// Change owner.
	pMeshDataContainer->pBoneHierarchyList = nullptr;
	pMeshDataContainer->pBoneOffsetMatrixList = nullptr;

	// Set Init Clip.
	wcscpy_s(_tCurAnimator.wcName, wcIdleClipName);

	return AK_TRUE;
}

void Animation::Update()
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

Matrix Animation::GetBoneTrnasformAtID(AkU32 uBoneID)
{
	Matrix* pBoneTransforms = GetBoneTransforms();
	return pBoneTransforms[uBoneID];
}

Matrix* Animation::GetBoneTransforms()
{
	Matrix* pToParentTransform = new Matrix[96];

	AnimationClip_t* pCurClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(_tCurAnimator.wcName) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pCurClip, 1, _tCurAnimator.wcName, uKeySize))
	{
		__debugbreak();
	}

	if (!wcslen(_tNextAnimator.wcName))
		wcscpy_s(_tNextAnimator.wcName, _tCurAnimator.wcName);

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

					//pCurTransform[i] = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qQuat) * Matrix::CreateTranslation(vPos);

					vCurScale = vScale;
					qCurQuat = qQuat;
					vCurPos = vPos;
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

					//pNextTransform[i] = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qQuat) * Matrix::CreateTranslation(vPos);

					vNextScale = vScale;
					qNextQuat = qQuat;
					vNextPos = vPos;
				}
			}

			const AkF32 fRatio = _fChangedTime / _fBlendTime;

			// pToParentTransform[i] = pCurTransform[i] * (1.0f - fRatio) + pNextTransform[i] * fRatio;

			Vector3 vScale = DirectX::XMVectorLerp(vCurScale, vNextScale, fRatio);
			Quaternion qQuat = DirectX::XMQuaternionSlerp(qCurQuat, qNextQuat, fRatio);
			Vector3 vPos = DirectX::XMVectorLerp(vCurPos, vNextPos, fRatio);

			pToParentTransform[i] = Matrix::CreateScale(vScale) * Matrix::CreateFromQuaternion(qQuat) * Matrix::CreateTranslation(vPos);
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

void Animation::SetEndCallBack(const wchar_t* wcClipname, Actor* pActor, AnimationClip_t::CALL_BACK pCallBack)
{
	AnimationClip_t* pClip = nullptr;
	AkU32 uKeySize = (AkU32)wcslen(wcClipname) * sizeof(wchar_t);
	if (!HT_Find(_pAnimationClipTable, (void**)&pClip, 1, wcClipname, uKeySize))
	{
		__debugbreak();
	}

	pClip->pCallBack = pCallBack;
	pClip->pActor = pActor;
}

AnimationClip_t* Animation::ReadClip(const wchar_t* wcBasePath, const wchar_t* wcFilename)
{
	ModelImporter tModelImporter = { };

	tModelImporter.LoadAnimation(wcBasePath, wcFilename, _uBoneNum);

	AnimationClip_t* pAnimClip = tModelImporter.GetAnimationClip();

	AddAnimationClip(pAnimClip, wcFilename);

	// Set Max Frame.
	AkU32 uMaxKeyFrame = 0;
	for (AkU32 i = 0; i < pAnimClip->uNumBoneAnimation; i++)
	{
		uMaxKeyFrame = max(uMaxKeyFrame, pAnimClip->pBoneAnimationList[i].uNumKeyFrame);
	}
	pAnimClip->uMaxKeyFrame = uMaxKeyFrame;

	// For Delete Clip.
	pAnimClip->tLink.pData = pAnimClip;
	LL_PushBack(&_pClipHead, &_pClipTail, &pAnimClip->tLink);

	return pAnimClip;
}

void Animation::PlayClip(const wchar_t* wcClipname, ANIM_CLIP_STATE eState, AkF32 fSpeed, AkF32 fBlendTime)
{
	_fChangedTime = 0.0f;

	_bIsChanging = AK_TRUE;

	_tCurAnimator.eAnimState = ANIM_CLIP_STATE::STOP;
	_tNextAnimator.eAnimState = eState;
	_tNextAnimator.uCurFrame = 0;
	_tNextAnimator.uNextFrame = 1;

	wcscpy_s(_tNextAnimator.wcName, wcClipname);

	_fAnimScale = fSpeed;
	_fBlendTime = fBlendTime;
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

void Animation::UpdateAnimator(Animator_t* pAnimator)
{
	if (ANIM_CLIP_STATE::LOOP == pAnimator->eAnimState)
	{
		AnimationClip_t* pClip = nullptr;
		AkU32 uKeySize = (AkU32)wcslen(pAnimator->wcName) * sizeof(wchar_t);
		if (!HT_Find(_pAnimationClipTable, (void**)&pClip, 1, pAnimator->wcName, uKeySize))
		{
			__debugbreak();
		}

		pAnimator->fFrameWeight += (DT * (AkF32)pClip->uTickPerSecond * _fAnimScale);

		// Frame 이 증가할수록 애니메이션이 느려지는 현상 // TODO
		if (pAnimator->fFrameWeight >= 1.0f) // 프레임 사이를 보간
		{
			pAnimator->fFrameWeight = 0.0f;
			pAnimator->uCurFrame++;
			pAnimator->uNextFrame++;

			if (pAnimator->uNextFrame >= pClip->uMaxKeyFrame)
			{
				pAnimator->uCurFrame = 0;
				pAnimator->uNextFrame = 1;

				if (pClip->pCallBack)
					pClip->pCallBack(pClip->pActor);
			}
		}
	}
}

void Animation::CleanUp()
{
	if (_ppBoneName)
	{
		for (AkU32 i = 0; i < _uBoneNum; i++)
		{
			free(_ppBoneName[i]);
			_ppBoneName[i] = nullptr;
		}
		free(_ppBoneName);
		_ppBoneName = nullptr;
	}
	if (_pFinalTransforms)
	{
		delete[] _pFinalTransforms;
		_pFinalTransforms = nullptr;
	}
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
		List_t* pCur = _pClipHead;
		List_t* pNext = nullptr;
		while (pCur != nullptr)
		{
			pNext = pCur->pNext;
			AnimationClip_t* pClip = (AnimationClip_t*)pCur->pData;
			if (pClip)
				delete[] pClip;

			pCur = pNext;
		}

		HT_DeleteAll(_pAnimationClipTable);
		HT_DestroyHashTable(_pAnimationClipTable);
		_pAnimationClipTable = nullptr;
	}
}