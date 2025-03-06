#include "pch.h"
#include "EditorModel.h"
#include "Camera.h"
#include "Application.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Actor.h"
#include "GameInput.h"
#include "Collider.h"
#include "Model.h"
#include "TextUI.h"

UEditorModel::UEditorModel()
{
}

UEditorModel::~UEditorModel()
{
	CleanUp();
}

AkBool UEditorModel::Initialize(UApplication* pApp)
{
	if (!UEditor::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	CreateEditorCamera();

	//_pBackGroundUI = new UBackGroundUI;
	//if (!_pBackGroundUI->Initialize(pApp))
	//{
	//	__debugbreak();
	//	return AK_FALSE;
	//}

	//_pBtnUI = new UBtnUI;
	//if (!_pBtnUI->Initialize(pApp))
	//{
	//	__debugbreak();
	//	return AK_FALSE;
	//}

	_pSystemTextUI = new UTextUI;
	if (!_pSystemTextUI->Initialize(pApp, 256, 32, L"Consolas", 12))
	{
		__debugbreak();
		return AK_FALSE;
	}

	_pEditorTextUI = new UTextUI;
	if (!_pEditorTextUI->Initialize(pApp, 256, 256, L"Consolas", 12))
	{
		__debugbreak();
		return AK_FALSE;
	}



	return AK_TRUE;
}

void UEditorModel::BeginEditor()
{
	ShowCursor(AK_TRUE);

	USceneManager* pSceneManager = GetApp()->GetSceneManager();
	UScene* pScene = pSceneManager->GetCurrentScene();
	GameObjContainer_t* pGameObjContainer = pScene->GetGroupObject(GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_PLAYER);
	UActor* pPlayer = (UActor*)pGameObjContainer->pGameObjHead->pData;
	UEditorCamera* pEditorCam = GetEditorCamera();

	Vector3 vPlayerPos = pPlayer->GetPosition();
	Vector3 vRelativePos = Vector3(0.0f, 0.0f, -3.0f);

	Vector3 vCamPos = vPlayerPos + vRelativePos;

	pEditorCam->SetCameraDirection(0.0f, 0.0f, 0.0f);

	pEditorCam->SetCameraPosition(vCamPos.x, vCamPos.y, vCamPos.z);
}

void UEditorModel::EndEditor()
{
}

void UEditorModel::Update(const AkF32 fDeltaTime)
{
	UEditorCamera* pEditorCam = GetEditorCamera();
	UGameInput* pGameInput = GetApp()->GetGameInput();

	if (pGameInput->KeyFirstDown(KEY_INPUT_F))
	{
		_bEnableCam = !_bEnableCam;
	}

	if (_bEnableCam)
	{
		pEditorCam->Update(fDeltaTime);
	}

	ProcessMousePicking();

	AkI32 iTextWidth = 0;
	AkI32 iTextHeight = 0;
	wchar_t wcText[128] = {};
	AkU32 uTxtLen = swprintf_s(wcText, L"Model edit mode");

	_pSystemTextUI->WriteText(wcText);

	if (_bEnableEditorUI)
	{
		IRenderer* pRenderer = GetApp()->GetRenderer();
		uTxtLen = swprintf_s(wcText, L"%s\n\nPos: %lf %lf %lf", _pPickingObj->GetName(), _pPickingObj->GetPosition().x, _pPickingObj->GetPosition().y, _pPickingObj->GetPosition().z);

		_pEditorTextUI->WriteText(wcText);
	}
}

void UEditorModel::ProcessMousePicking()
{
	//UApplication* pApp = GetApp();
	//IRenderer* pRenderer = pApp->GetRenderer();
	//UGameInput* pGameInput = pApp->GetGameInput();
	//AkF32 fNdcX = pApp->GetNdcX();
	//AkF32 fNdcY = pApp->GetNdcY();

	//static UActor* pActiveActor = nullptr;
	//static Vector3 vPrevHitPos = Vector3(0.0f);
	//static AkF32 fPrevRatio = 0.0f;
	//static Vector3 vPrevVector = Vector3(0.0f);

	//Quaternion qQuat = Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
	//AkF32 fDist = 0.0f;
	//Vector3 vHitPos = Vector3(0.0f);
	//AkF32 fRatio = 0.0f;
	//Vector3 vDragTranslation = Vector3(0.0f);
	//Vector3 vPickPoint = Vector3(0.0f);

	//if (pGameInput->LeftBtnHold() || pGameInput->RightBtnHold())
	//{
	//	Vector3 vWorldNear = pRenderer->GetWorldNearPosition(fNdcX, fNdcY);
	//	Vector3 vWorldFar = pRenderer->GetWorldFarPosition(fNdcX, fNdcY);
	//	if (!pActiveActor)
	//	{
	//		UActor* pNewActor = PickClosets(&vPickPoint, &fDist, &fRatio);
	//		if (pNewActor)
	//		{
	//			wprintf_s(L"Newly selected model: %s\n", pNewActor->GetName());

	//			_bEnableEditorUI = AK_TRUE;

	//			pActiveActor = pNewActor;
	//			vPrevHitPos = vPickPoint;
	//			vHitPos = vPickPoint;
	//			fPrevRatio = fRatio;

	//			UCollider* pCollider = pActiveActor->GetPickingCollider();

	//			vPrevVector = vHitPos - pCollider->GetBoundingSphere()->Center;

	//			_pPickingObj = pActiveActor; // For GUI Control.
	//		}
	//	}
	//	else
	//	{
	//		if (pGameInput->LeftBtnHold()) // Left Btn
	//		{
	//			UCollider* pCollider = pActiveActor->GetPickingCollider();

	//			AkBool bIntersect = ProcessMousePickingWithSphere(pCollider, &vPickPoint, &fDist, &fRatio);
	//			if (bIntersect)
	//			{
	//				vHitPos = vPickPoint;
	//			}

	//			Vector3 vCurVector = vHitPos - pCollider->GetBoundingSphere()->Center;
	//			vCurVector.Normalize();

	//			AkF32 fTheta = (AkF32)acos(vPrevVector.Dot(vCurVector));
	//			if (fTheta > 3.141592f / 180.0f * 3.0f)
	//			{
	//				Vector3 vAxis = vPrevVector.Cross(vCurVector);
	//				vAxis.Normalize();

	//				qQuat = Quaternion::CreateFromAxisAngle(vAxis, fTheta);

	//				vPrevVector = vCurVector;
	//			}
	//		}
	//		else // Right Btn
	//		{
	//			Vector3 vNewHitPos = vWorldNear + fPrevRatio * (vWorldFar - vWorldNear);
	//			if ((vNewHitPos - vPrevHitPos).Length() > 1e-3)
	//			{
	//				vDragTranslation = vNewHitPos - vPrevHitPos;
	//				vPrevHitPos = vNewHitPos;
	//			}
	//		}
	//	}
	//}
	//else
	//{
	//	pActiveActor = nullptr;
	//}

	//if (pActiveActor)
	//{
	//	Vector3 vTranslation = pActiveActor->GetPosition();

	//	pActiveActor->SetPosition(0.0f, 0.0f, 0.0f);

	//	vTranslation += vDragTranslation;

	//	// Matrix mRot = pActiveActor->GetModel(pActiveActor->GetModelContextIndex())->GetRotationMat();

	//	// pActiveActor->GetModel(pActiveActor->GetModelContextIndex())->SetRotationMat(mRot * Matrix::CreateFromQuaternion(qQuat));

	//	pActiveActor->SetPosition(vTranslation.x, vTranslation.y, vTranslation.z);

	//   // 여기서 작업을 해도 MoveModel을 통해 World Matrix 가 갱신됨.
	//}
}

UActor* UEditorModel::PickClosets(Vector3* pHitPos, AkF32* pMinDist, AkF32* pRatio)
{
	USceneManager* pSceneManager = GetApp()->GetSceneManager();
	UScene* pScene = pSceneManager->GetCurrentScene();
	GameObjContainer_t** ppAllGameObj = pScene->GetAllGameObject();
	UActor* pMinActor = nullptr;
	Vector3 vMinHitPos = Vector3(0.0f);
	AkF32 fMinDist = AK_MAX_F32;
	AkF32 fMinRatio = 0.0f;

	//for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; i++)
	//{
	//	GameObjContainer_t* pGroupObj = ppAllGameObj[i];
	//	if (!pGroupObj)
	//	{
	//		continue;
	//	}

	//	List_t* pCur = pGroupObj->pGameObjHead;
	//	while (pCur != nullptr)
	//	{
	//		UActor* pActor = (UActor*)pCur->pData;
	//		UCollider* pCollider = pActor->GetPickingCollider();
	//		Vector3 vHitPos = Vector3(0.0f);
	//		AkF32 fDist = 0.0f;
	//		AkF32 fRatio = 0.0f;
	//		AkBool bIsPicked = AK_FALSE;

	//		// Collider 가 없으면 Picking 불가능
	//		if (nullptr == pCollider)
	//		{
	//			pCur = pCur->pNext;
	//			continue;
	//		}

	//		bIsPicked = ProcessMousePickingWithSphere(pCollider, &vHitPos, &fDist, &fRatio);

	//		if (bIsPicked && fDist < fMinDist)
	//		{
	//			pMinActor = pActor;
	//			vMinHitPos = vHitPos;
	//			fMinDist = fDist;
	//			fMinRatio = fRatio;
	//		}

	//		pCur = pCur->pNext;
	//	}
	//}

	//*pHitPos = vMinHitPos;
	//*pMinDist = fMinDist;
	//*pRatio = fMinRatio;

	return pMinActor;
}

AkBool UEditorModel::ProcessMousePickingWithSphere(UCollider* pCollider, Vector3* pHitPos, AkF32* pDist, AkF32* pRatio)
{
	//UApplication* pApp = GetApp();
	//IRenderer* pRenderer = pApp->GetRenderer();
	//AkF32 fNdcX = pApp->GetNdcX();
	//AkF32 fNdcY = pApp->GetNdcY();

	//Vector3 vHitPos = Vector3(0.0f);
	//AkF32 fHitDist = 0.0f;
	//AkF32 fRatio = 0.0f;

	//AkBool bIntersect = pRenderer->MousePickingToSphere(pCollider->GetBoundingSphere(), fNdcX, fNdcY, &vHitPos, &fHitDist, &fRatio);

	//*pHitPos = vHitPos;
	//*pDist = fHitDist;
	//*pRatio = fRatio;

	//return bIntersect;

	return AK_TRUE;
}

void UEditorModel::Render()
{
	IRenderer* pRenderer = GetApp()->GetRenderer();

	//_pSystemTextUI->Render(300, 10, 1.0f, 1.0f, nullptr, 0.0f, nullptr);

	//if (_bEnableEditorUI)
	//{
	//	_pBackGroundUI->Render(10, 64, 1.0f, 1.0f, 0.0001f);

	//	_pBtnUI->Render(20, 64, 0.25f, 0.25f, 0.0f);

	//	_pEditorTextUI->Render(10 + 3, 64 + 3, 1.0f, 1.0f, nullptr, 0.0f, nullptr);
	//}
}

void UEditorModel::CleanUp()
{
	if (_pEditorTextUI)
	{
		delete _pEditorTextUI;
		_pEditorTextUI = nullptr;
	}
	if (_pSystemTextUI)
	{
		delete _pSystemTextUI;
		_pSystemTextUI = nullptr;
	}
	if (_pBtnUI)
	{
		delete _pBtnUI;
		_pBtnUI = nullptr;
	}
	if (_pBackGroundUI)
	{
		delete _pBackGroundUI;
		_pBackGroundUI = nullptr;
	}

	DestroyEditorCamera();
}







