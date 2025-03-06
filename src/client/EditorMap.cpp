#include "pch.h"
#include "EditorMap.h"
#include "Camera.h"
#include "Application.h"
#include "SceneManager.h"
#include "Scene.h"
#include "LandScape.h"
#include "GameInput.h"
#include "GeometryGenerator.h"
#include "TextUI.h"

#include "WorldMap.h"
#include "Collider.h"

UEditorMap::UEditorMap()
{
}

UEditorMap::~UEditorMap()
{
	CleanUp();
}

AkBool UEditorMap::Initialize(UApplication* pApp)
{
	if (!UEditor::Initialize(pApp))
	{
		__debugbreak();
		return AK_FALSE;
	}

	CreateEditorCamera();

	_pSystemTextUI = new UTextUI;
	if (!_pSystemTextUI->Initialize(pApp, 512, 32, L"Consolas", 12))
	{
		__debugbreak();
		return AK_FALSE;
	}

	return AK_TRUE;
}

void UEditorMap::BeginEditor()
{
	ShowCursor(AK_TRUE);

	UEditorCamera* pEditorCam = GetEditorCamera();

	pEditorCam->SetCameraDirection(0.0f, DirectX::XM_PIDIV2, 0.0f);

	pEditorCam->SetCameraPosition(0.0f, 15.0f, 0.0f);
}

void UEditorMap::EndEditor()
{
}

void UEditorMap::Update(const AkF32 fDeltaTime)
{
	USceneManager* pSceneManager = GetApp()->GetSceneManager();
	UScene* pScene = pSceneManager->GetCurrentScene();
	AkU32 uGameObjNum = pScene->GetGameObjectNum();
	ULandScape* uLandScapeCell = pScene->GetLandScape();
	// AkU32 uLandScapeCellCount = uLandScapeCell->GetCellCount();
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

	UpdateMapObject();
	UpdateText();
}

void UEditorMap::Render()
{
	//_pSystemTextUI->Render(300, 10, 1.0f, 1.0f, nullptr, 0.0f, nullptr);
}

void UEditorMap::CleanUp()
{
	if (_pSystemTextUI)
	{
		delete _pSystemTextUI;
		_pSystemTextUI = nullptr;
	}

	DestroyEditorCamera();
}

void UEditorMap::UpdateMapObject()
{
	UApplication* pApp = GetApp();
	IRenderer* pRenderer = pApp->GetRenderer();
	UGameInput* pGameInput = pApp->GetGameInput();
	AkF32 fNdcX = pApp->GetClampNdcX();
	AkF32 fNdcY = pApp->GetClampNdcY();

	DirectX::SimpleMath::Plane tPlane(Vector3(0.0f, 1.0f, 0.0f), 0.0f); // xz - plane
	Vector3 vHitPos = Vector3(0.0f);
	AkF32 fHitDist = 0.0f;
	AkF32 fRatio = 0.0f;
	if (pGameInput->LeftBtnDown())
	{
		if (pRenderer->MousePickingToPlane(&tPlane, fNdcX, fNdcY, &vHitPos, &fHitDist, &fRatio))
		{
			_vLeftTopPos = vHitPos;
		}
	}
	if (pGameInput->LeftBtnUp())
	{
		if (pRenderer->MousePickingToPlane(&tPlane, fNdcX, fNdcY, &vHitPos, &fHitDist, &fRatio))
		{
			_vRightBottomPos = vHitPos;

			CreateBox();
		}
	}
}

void UEditorMap::UpdateText()
{
	USceneManager* pSceneManager = GetApp()->GetSceneManager();
	UScene* pScene = pSceneManager->GetCurrentScene();
	AkU32 uGameObjNum = pScene->GetGameObjectNum();
	UEditorCamera* pEditorCam = GetEditorCamera();

	Vector3 vCamPos = pEditorCam->GetCameraPosition();
	Vector3 vCamDir = pEditorCam->GetCameraDirection();

	wchar_t wcText[128] = {};
	AkU32 uTxtLen = swprintf_s(wcText, L"Edit mode\nobj:%u cam pos/dir:%lf %lf %lf/%lf %lf %lf", uGameObjNum, vCamPos.x, vCamPos.y, vCamPos.z, vCamDir.x, vCamDir.y, vCamDir.z);

	_pSystemTextUI->WriteText(wcText);
}

void UEditorMap::CreateBox()
{
	USceneManager* pSceneManager = GetApp()->GetSceneManager();
	UScene* pScene = pSceneManager->GetCurrentScene();

	AkF32 fScalsX = abs(_vRightBottomPos.x - _vLeftTopPos.x);
	AkF32 fScaleZ = abs(_vRightBottomPos.z - _vLeftTopPos.z);

	Vector3 vScale = Vector3(fScalsX, 1.0f, fScaleZ);

	Vector3 vPos = (_vRightBottomPos + _vLeftTopPos) * 0.5f;
	vPos.y = 0.0f;

	AkU32 uMeshDataNum = 0;
	MeshData_t* pMeshData = UGeometryGenerator::MakeCube(&uMeshDataNum, 0.5f);
	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		wcscpy_s(pMeshData[i].wcAlbedoTextureFilename, L"../../assets/asset_test_01.dds");
	}

	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		for (AkU32 j = 0; j < pMeshData[i].uVerticeNum; j++)
		{
			pMeshData[i].pVertices[j].vPosition = Vector3::Transform(pMeshData[i].pVertices[j].vPosition, Matrix::CreateScale(vScale) * Matrix::CreateTranslation(vPos));
		}
	}
}
