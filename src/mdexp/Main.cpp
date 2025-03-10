#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>
#include "ModelImporter.h"
#include "ModelExporter.h"
#include "Model.h"
#include "SkinnedModel.h"
#include "Utils.h"
#include "GeometryGenerator.h"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include <ImGuizmo.h>

#include <shlwapi.h>
#include <atlconv.h>
#pragma comment(lib, "Shlwapi.lib")

extern ImGuiContext* GImGui;

using namespace std;

/*
================
Global Variable
================
*/

HWND g_hWnd = nullptr;
unsigned int g_uScreenWidth = 0;
unsigned int g_uScreenHeight = 0;
bool g_bEnableDebugLayer = false;
bool g_bEnableGBV = false;
HMODULE g_hRendererDLL = nullptr;
IRenderer* g_pRenderer = nullptr;
void* g_pIrradianceTexHandle;
void* g_pSpecularTexHandle;
void* g_pBrdfTexHandle;
AkF32 g_fIBLStrength = 0.35f;
vector<Model*> g_vecAllModelList;
Model* g_pCharacterModel;
vector<Model*> g_vecWeaponModel;
float g_fFps;
bool g_bKeyDown[256];
float g_fCamSpeed = 1.0f;
float g_fNdcX;
float g_fNdcY;
int g_iMouseX;
int g_iMouseY;
bool g_bFPV;
bool g_bEnableAnim;
bool g_bAttachWeaponLeftHand;
bool g_bAttachWeaponRightHand;
Vector3 g_vInitCamDir = Vector3(0.0f, 0.0f, 1.0f);
Vector3 g_vCurCamDir;
Vector3 g_vCurCamRight;
Matrix g_mViewMat;
Matrix g_mProjMat;
bool g_bFileOpen;
string g_strOpenFilePath;
string g_strOpenFileName;
vector<pair<ModelImporter*, bool>> g_vecModelImp;
string g_strOutputFilePath = "../../assets/model/";
Model* g_pPrevPickModel;
bool g_bLBtnDown;
bool g_bChecked;
bool g_bCharacterDelete;
bool g_bFileSavingEnd;

/*
=================
Func declaration
=================
*/

void RunModelExporter();
void RunModelViewr();

void InitRenderer();
void InitIBLResource(const wchar_t* wcIrradianceFilename, const wchar_t* wcSpecularFilename, const wchar_t* wcBrdfFilename);
void InitCamera();
void InitScene();
void Update(const float fDT);
void UpdateMouse(int iMouseX, int iMouseY);
void UpdateMousePicking();
void UpdateCamera(const float fDT);
void UpdateGui();
void Render();
void ProcessMeshData(MeshData_t* pMeshData, int iMeshCount, std::vector<MeshData>& meshes, bool bIsAnim = false, AnimationData* pAnimData = nullptr);
void DestroyMeshData(MeshData_t* pMeshData, int iMeshCount);
Model* CreateModel(MeshData_t* pMeshData, int uMeshDataNum);
SkinnedModel* CreateModel(MeshData_t* pMeshData, int uMeshDataNum, AnimationData* pAnimData);
ModelImporter* CreateModelImporter(string strOpenFilePath, string strOpenFileName, string strOutputFilrPath, bool bNormalRevert, bool bAnim = true);
void CleanUpRenderer();
void CleanUpIBLResource();
void CleanUpModelList();
void CleanUpModelImporterList();

Model* GetClosestObj();

void OpenCharacterFile();
void OpenWeaponFile();
void SaveModelFile();
void SavaAnimationFile();

string ToString(wstring wStr)
{
	string temp;
	temp.assign(wStr.begin(), wStr.end());
	return temp;
}

LRESULT WndProc(HWND hWnd, AkU32 uMsg, WPARAM wParam, LPARAM lParam);

/*
=================
Entry Point
=================
*/

int main()
{
	const wchar_t wcClassName[] = L"Window Application";
	const wchar_t wcWindowName[] = L"Model Viewer";

	WNDCLASS tWc = { };
	tWc.lpfnWndProc = WndProc;
	tWc.hInstance = ::GetModuleHandle(nullptr);
	tWc.lpszClassName = wcClassName;
	RegisterClass(&tWc);

	HWND hWnd = ::CreateWindowEx(0, wcClassName, wcWindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, tWc.hInstance, nullptr);
	g_hWnd = hWnd;

	::ShowWindow(hWnd, SW_SHOW);

	RECT tRect = {};
	::GetClientRect(g_hWnd, &tRect);
	g_uScreenWidth = tRect.right - tRect.left;
	g_uScreenHeight = tRect.bottom - tRect.top;

	// Init Renderer.
	InitRenderer();

	// Init IBL.
	InitIBLResource(L"../../assets/skybox/SampleDiffuseHDR.dds", L"../../assets/skybox/SampleSpecularHDR.dds", L"../../assets/skybox/SampleBrdf.dds");

	// Init Camera.
	InitCamera();

	// Init Scene
	InitScene();

	// Run.
	MSG tMsg = { };
	while (true)
	{
		if (PeekMessage(&tMsg, nullptr, 0, 0, PM_REMOVE))
		{
			if (tMsg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&tMsg);
			DispatchMessage(&tMsg);
		}
		else
		{
			// Start the Dear ImGui frame
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuiIO& io = ImGui::GetIO();

			float fDT = 1 / io.Framerate;
			float fFps = io.Framerate;
			g_fFps = fFps;

			Update(fDT);

			Render();

			ImGui::Render();

			g_pRenderer->EndRender();
			g_pRenderer->Present();

			if (g_bCharacterDelete)
			{
				if (g_pCharacterModel)
				{
					int idx = 0;
					for (auto& e : g_vecAllModelList)
					{
						if (e == g_pCharacterModel)
						{
							g_vecAllModelList.erase(g_vecAllModelList.begin() + idx);
							break;
						}
						idx++;
					}

					delete g_pCharacterModel;
					g_pCharacterModel = nullptr;
					g_bCharacterDelete = false;
				}
			}
		}
	}

	CleanUpModelList();
	CleanUpModelImporterList();
	CleanUpIBLResource();
	CleanUpRenderer();

	return 0;
}

void RunModelExporter()
{
}

void RunModelViewr()
{
}

void InitRenderer()
{
	const wchar_t* wcRendererDLLFilename = nullptr;

#if defined(_M_AMD64)

#if defined(_DEBUG) || defined(DEBUG)
	wcRendererDLLFilename = L"akatsuki_renderer_x64d.dll";
#else
	wcRendererDLLFilename = L"akatsuki_renderer_x64.dll";
#endif

#elif defined(_M_IX86)

#if defined(_DEBUG) | defined(DEBUG)
	wcRendererDLLFilename = L"akatsuki_renderer_x86d.dll";
#else
	wcRendererDLLFilename = L"akatsuki_renderer_x86.dll";
#endif

#endif

	wchar_t wcErrorText[128] = {};
	AkU32 uErrorCode = 0;

	g_hRendererDLL = ::LoadLibrary(wcRendererDLLFilename);
	if (!g_hRendererDLL)
	{
		uErrorCode = ::GetLastError();
		swprintf_s(wcErrorText, L"Failed LoadLibrary[%s] - Error Code: %u \n", wcRendererDLLFilename, uErrorCode);
		::MessageBox(g_hWnd, wcErrorText, L"Error", MB_OK);
		__debugbreak();
	}

	DLL_CreateInstanceFuncPtr pDLL_CreateInstance = reinterpret_cast<DLL_CreateInstanceFuncPtr>(::GetProcAddress(g_hRendererDLL, "DLL_CreateInstance"));
	pDLL_CreateInstance(reinterpret_cast<void**>(&g_pRenderer));

	if (!g_pRenderer->Initialize(g_hWnd, g_bEnableDebugLayer, g_bEnableGBV))
	{
		__debugbreak();
	}

	// Use ImGui.
	g_pRenderer->BindImGui((void**)&GImGui);
}

void InitIBLResource(const wchar_t* wcIrradianceFilename, const wchar_t* wcSpecularFilename, const wchar_t* wcBrdfFilename)
{
	g_pIrradianceTexHandle = g_pRenderer->CreateCubeMapTexture(wcIrradianceFilename);
	g_pSpecularTexHandle = g_pRenderer->CreateCubeMapTexture(wcSpecularFilename);
	g_pBrdfTexHandle = g_pRenderer->CreateTextureFromFile(wcBrdfFilename, AK_FALSE);

	g_pRenderer->BindIBLTexture(g_pIrradianceTexHandle, g_pSpecularTexHandle, g_pBrdfTexHandle);
	g_pRenderer->SetIBLStrength(g_fIBLStrength);
}

void InitCamera()
{
	g_pRenderer->SetCameraPosition(0.0f, 0.0f, -3.0f);

	Vector3 vUp = Vector3(0.0f, 1.0f, 0.0f);
	g_vCurCamDir = g_vInitCamDir;
	g_vCurCamRight = vUp.Cross(g_vCurCamDir);
}

void InitScene()
{
	// Create the ground.
	{
		int iMeshDataNum = 1;
		MeshData pSquare = GeometryGenerator::MakeSquare(15.0f, Vector2(15.0f));
		MeshData_t* pMeshData = new MeshData_t;
		pMeshData->pVertices = new Vertex_t[pSquare.vertices.size()];
		pMeshData->uVerticeNum = pSquare.vertices.size();
		pMeshData->pIndices = new unsigned int[pSquare.indices.size()];
		pMeshData->uIndicesNum = pSquare.indices.size();
		wcscpy_s(pMeshData->wcAlbedoTextureFilename, L"../../assets/map/Poliigon_TilesCeramicWhite_6956_BaseColor.dds");
		wcscpy_s(pMeshData->wcNormalTextureFilename, L"../../assets/map/Poliigon_TilesCeramicWhite_6956_Normal.dds");
		wcscpy_s(pMeshData->wcRoughnessTextureFilename, L"../../assets/map/Poliigon_TilesCeramicWhite_6956_Roughness.dds");
		wcscpy_s(pMeshData->wcMetallicTextureFilename, L"../../assets/map/Poliigon_TilesCeramicWhite_6956_Metallic.dds");
		wcscpy_s(pMeshData->wcAoTextureFilename, L"../../assets/map/Poliigon_TilesCeramicWhite_6956_AmbientOcclusion.dds");
		memcpy(pMeshData->pVertices, pSquare.vertices.data(), sizeof(Vertex_t) * pSquare.vertices.size());
		memcpy(pMeshData->pIndices, pSquare.indices.data(), sizeof(unsigned int) * pSquare.indices.size());
		Model* pGround = CreateModel(pMeshData, iMeshDataNum);
		pGround->Name = "Ground";
		pGround->Pickable = false;
		Matrix& mWorld = pGround->GetWorldRow();
		mWorld = Matrix::CreateRotationX(DirectX::XM_PIDIV2) * Matrix::CreateTranslation(Vector3(0.0f, -0.5f, 0.0f));
		if (pMeshData)
		{
			if (pMeshData->pVertices)
			{
				delete pMeshData->pVertices;
				pMeshData->pVertices = nullptr;
			}
			if (pMeshData->pIndices)
			{
				delete pMeshData->pIndices;
				pMeshData->pIndices = nullptr;
			}
			delete pMeshData;
			pMeshData = nullptr;
		}
	}
}

void CleanUpRenderer()
{
	g_pRenderer->UnBindImGui();

	if (g_pRenderer)
	{
		g_pRenderer->Release();
		g_pRenderer = nullptr;
	}
}

void CleanUpIBLResource()
{
	g_pRenderer->DestroyTexture(g_pBrdfTexHandle);
	g_pRenderer->DestroyTexture(g_pSpecularTexHandle);
	g_pRenderer->DestroyTexture(g_pIrradianceTexHandle);
}

void CleanUpModelList()
{
	for (auto& m : g_vecAllModelList)
	{
		delete m;
		m = nullptr;
	}
	g_vecAllModelList.clear();
}

void CleanUpModelImporterList()
{
	for (auto& e : g_vecModelImp)
	{
		delete e.first;
		e.first = nullptr;
	}
	g_vecModelImp.clear();
}

void OpenCharacterFile()
{
	OPENFILENAME ofn = {};
	TCHAR filePathName[100] = L"";
	TCHAR lpstrFile[100] = L"";
	static TCHAR filter[] = L"모든 파일\0*.*\0텍스트 파일\0*.txt\0fbx 파일\0*.fbx";

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = lpstrFile;
	ofn.nMaxFile = 100;
	ofn.lpstrInitialDir = L".";

	if (GetOpenFileName(&ofn) != 0)
	{
		wsprintf(filePathName, L"%s 파일을 열겠습니까?", ofn.lpstrFile);
		MessageBox(g_hWnd, filePathName, L"열기 선택", MB_OK);

		g_strOpenFileName = ToString(PathFindFileName(ofn.lpstrFile));
		g_strOpenFilePath = "./data/";
		// PathRemoveFileSpec(ofn.lpstrFile);
		SetCurrentDirectory(L"../");

		ModelImporter* importer = CreateModelImporter(g_strOpenFilePath, g_strOpenFileName, g_strOutputFilePath, false);

		PathRemoveExtension(ofn.lpstrFile);
		g_strOpenFileName = ToString(PathFindFileName(ofn.lpstrFile));

		// Init Model.
		{
			int iMeshCount = g_vecModelImp.back().first->m_meshes.size();
			MeshData_t* pMeshData = new MeshData_t[iMeshCount];
			ProcessMeshData(pMeshData, iMeshCount, g_vecModelImp.back().first->m_meshes, true, &g_vecModelImp.back().first->m_aniData);

			Model* pModel = CreateModel(pMeshData, iMeshCount, &g_vecModelImp.back().first->m_aniData);
			pModel->Picked = false;
			pModel->Name = "Character";
			pModel->Path = g_strOutputFilePath;
			pModel->FileName = g_strOpenFileName;
			pModel->Shadow = true;
			pModel->Pickable = false;

			g_pCharacterModel = pModel;

			DestroyMeshData(pMeshData, iMeshCount);
		}
	}
}

void OpenWeaponFile()
{
	OPENFILENAME ofn = {};
	TCHAR filePathName[100] = L"";
	TCHAR lpstrFile[100] = L"";
	static TCHAR filter[] = L"모든 파일\0*.*\0텍스트 파일\0*.txt\0fbx 파일\0*.fbx";

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = lpstrFile;
	ofn.nMaxFile = 100;
	ofn.lpstrInitialDir = L".";

	if (GetOpenFileName(&ofn) != 0)
	{
		wsprintf(filePathName, L"%s 파일을 열겠습니까?", ofn.lpstrFile);
		MessageBox(g_hWnd, filePathName, L"열기 선택", MB_OK);

		g_strOpenFileName = ToString(PathFindFileName(ofn.lpstrFile));
		g_strOpenFilePath = "./data/";
		// PathRemoveFileSpec(ofn.lpstrFile);
		SetCurrentDirectory(L"../");

		ModelImporter* importer = CreateModelImporter(g_strOpenFilePath, g_strOpenFileName, g_strOutputFilePath, false, false);

		PathRemoveExtension(ofn.lpstrFile);
		g_strOpenFileName = ToString(PathFindFileName(ofn.lpstrFile));

		// Init Model.
		{
			int iMeshCount = g_vecModelImp.back().first->m_meshes.size();
			MeshData_t* pMeshData = new MeshData_t[iMeshCount];
			ProcessMeshData(pMeshData, iMeshCount, g_vecModelImp.back().first->m_meshes);

			Model* pModel = CreateModel(pMeshData, iMeshCount);
			pModel->Name = "Weapon";
			pModel->Path = g_strOutputFilePath;
			pModel->FileName = g_strOpenFileName;
			pModel->Picked = true;
			pModel->IsWeapon = true;
			pModel->Shadow = true;

			g_vecWeaponModel.push_back(pModel);

			DestroyMeshData(pMeshData, iMeshCount);
		}
	}
}

void SaveModelFile()
{
	OPENFILENAME ofn = {};
	TCHAR filePathName[100] = L"";
	TCHAR lpstrFile[100] = L"";
	static TCHAR filter[] = L"모든 파일\0*.*\0모델 파일\0*.md3d\0애니매이션 파일\0*.anim";

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = lpstrFile;
	ofn.nMaxFile = 100;
	ofn.lpstrInitialDir = L".";
	if (GetSaveFileName(&ofn) != 0)
	{
		wsprintf(filePathName, L"%s 파일을 저장하겠습니까?", ofn.lpstrFile);
		MessageBox(g_hWnd, filePathName, L"저장 선택", MB_OK);

		// Save Model File.
		if (g_vecModelImp.back().second) // Bone 정보가 포함된 .md3d 파일
		{
			if (Write(*(g_vecModelImp.back().first), g_strOutputFilePath, g_pCharacterModel->FileName + ".md3d", g_vecModelImp.back().second))
			{
				g_bFileSavingEnd = true;
			}
			else
			{
				g_bFileSavingEnd = false;
			}
		}
		else // Bone 정보가 포함되지 않은 .md3d 파일
		{
			if (Write(*(g_vecModelImp.back().first), g_strOutputFilePath, g_vecWeaponModel.back()->FileName + ".md3d", g_vecModelImp.back().second))
			{
				g_bFileSavingEnd = true;
			}
			else
			{
				g_bFileSavingEnd = false;
			}
		}
	}
}

void SavaAnimationFile()
{
	OPENFILENAME ofn = {};
	TCHAR filePathName[100] = L"";
	TCHAR lpstrFile[100] = L"";
	static TCHAR filter[] = L"모든 파일\0*.*\0모델 파일\0*.md3d\0애니매이션 파일\0*.anim";

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = lpstrFile;
	ofn.nMaxFile = 100;
	ofn.lpstrInitialDir = L".";
	if (GetSaveFileName(&ofn) != 0)
	{
		wsprintf(filePathName, L"%s 파일을 저장하겠습니까?", ofn.lpstrFile);
		MessageBox(g_hWnd, filePathName, L"저장 선택", MB_OK);

		// Save Anim File.
		if (WriteAnim(*(g_vecModelImp.front().first), g_strOutputFilePath, g_pCharacterModel->FileName + ".anim"))
		{
			g_bFileSavingEnd = true;
		}
		else
		{
			g_bFileSavingEnd = false;
		}
	}
}

void Update(const float fDT)
{
	UpdateCamera(fDT);

	g_pRenderer->GetViewPorjMatrix(&g_mViewMat, &g_mProjMat);

	// 수정 필요!!.
	Model* pModel = nullptr;
	for (auto e : g_vecAllModelList)
	{
		if (e->Name == "Weapon")
		{
			pModel = e;
		}
	}

	for (const auto& m : g_vecAllModelList)
	{
		m->Update(fDT);
	}

	// Attach the weapon.
	if (g_pCharacterModel)
	{
		if (g_bAttachWeaponLeftHand && !g_bChecked)
		{
			((SkinnedModel*)(g_pCharacterModel))->AttachWeaponLeftHand(pModel);
			g_bChecked = true;
		}
		if (g_bAttachWeaponRightHand && !g_bChecked)
		{
			((SkinnedModel*)(g_pCharacterModel))->AttachWeaponRightHand(pModel);
			g_bChecked = true;
		}
	}

	UpdateMousePicking();

	UpdateGui();
}

void Render()
{
	g_pRenderer->UpdateCascadeOrthoProjMatrix();

	// Render Shadow Pass.
	for (AkU32 i = 0; i < 5; i++)
	{
		g_pRenderer->BeginCasterRenderPreparation();

		for (const auto& m : g_vecAllModelList)
		{
			if (m->Shadow)
				m->RenderShadow();
		}

		g_pRenderer->EndCasterRenderPreparation();
	}

	// Render Opaque.
	g_pRenderer->BeginRender();

	for (const auto& m : g_vecAllModelList)
	{
		m->Render();
	}
}

void UpdateMouse(int iMouseX, int iMouseY)
{
	float fNdcX = (float)iMouseX / g_uScreenWidth * 2.0f - 1.0f;
	float fNdcY = (float)iMouseY / g_uScreenHeight * -2.0f + 1.0f;

	g_fNdcX = std::clamp(fNdcX, -1.0f, 1.0f);
	g_fNdcY = std::clamp(fNdcY, -1.0f, 1.0f);
}

void UpdateMousePicking()
{
	Model* pClosestModel = nullptr;
	if (g_bLBtnDown)
	{
		pClosestModel = GetClosestObj();
	}
	else
	{
		pClosestModel = nullptr;
	}

	if (pClosestModel == nullptr)
	{
		return;
	}

	// 성능과 영향이 있는 로직.
	for (const auto& e : g_vecAllModelList)
	{
		e->Picked = false;
	}

	pClosestModel->Picked = true;
}

Model* GetClosestObj()
{
	Model* pTarget = nullptr;
	Vector3 vHitPos = Vector3(0.0f);
	float fHitDist = 0.0f;
	float fRatio = 0.0f;
	float fMinDist = FLT_MAX;
	for (const auto& e : g_vecAllModelList)
	{
		if (e->Pickable)
		{
			bool bPicking = g_pRenderer->MousePickingToBox(&e->BndBox, g_fNdcX, g_fNdcY, &vHitPos, &fHitDist, &fRatio);
			if (bPicking && fMinDist > fHitDist)
			{
				fMinDist = fHitDist;
				pTarget = e;
			}
		}
	}
	return pTarget;
}

void UpdateCamera(const float fDT)
{
	if (!g_bFPV)
	{
		return;
	}

	Vector3 vDeltaPos = Vector3(0.0f);
	Vector3 vUp = Vector3(0.0f, 1.0f, 0.0f);

	if (g_bKeyDown['W'])
	{
		vDeltaPos = g_vCurCamDir * (fDT * g_fCamSpeed);
	}
	if (g_bKeyDown['S'])
	{
		vDeltaPos = -g_vCurCamDir * (fDT * g_fCamSpeed);
	}
	if (g_bKeyDown['D'])
	{
		vDeltaPos = g_vCurCamRight * (fDT * g_fCamSpeed);
	}
	if (g_bKeyDown['A'])
	{
		vDeltaPos = -g_vCurCamRight * (fDT * g_fCamSpeed);
	}
	if (g_bKeyDown['Q'])
	{
		vDeltaPos = vUp * (fDT * g_fCamSpeed);
	}
	if (g_bKeyDown['E'])
	{
		vDeltaPos = -vUp * (fDT * g_fCamSpeed);
	}

	g_pRenderer->MoveCamera(vDeltaPos.x, vDeltaPos.y, vDeltaPos.z);

	// Camera 회전
	AkF32 fYaw = DirectX::XM_2PI * g_fNdcX;
	AkF32 fPitch = -DirectX::XM_PIDIV2 * g_fNdcY;

	g_pRenderer->RotateYawPitchRollCamera(fYaw, fPitch, 0.0f);

	g_vCurCamDir = Vector3::Transform(g_vInitCamDir, Matrix::CreateFromYawPitchRoll(fYaw, fPitch, 0.0f));
	g_vCurCamRight = vUp.Cross(g_vCurCamDir);
}

void UpdateGui()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_MenuBar;
	bool bOpen = true;
	ImGui::Begin("Model Exporter", nullptr, window_flags);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::Text("Ctrl Key Down : Model Delete");
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("01.Open Character Model", "Ctrl+"))
			{
				OpenCharacterFile();
			}
			if (ImGui::MenuItem("02.Open Weapon Model", "Ctrl+"))
			{
				OpenWeaponFile();
			}
			if (ImGui::MenuItem("03.Save Model File", "Ctrl+"))
			{
				SaveModelFile();
			}
			if (ImGui::MenuItem("04.Save Anim File", "Ctrl+"))
			{
				SavaAnimationFile();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	for (auto& e : g_vecAllModelList)
	{
		ImGui::Text(string("Current File: " + e->FileName).c_str());
	}
	if (g_bFileSavingEnd)
	{
		ImGui::Text("End File Save");
	}
	else
	{
		ImGui::Text("File Save State");
	}
	ImGui::End();
}

void ProcessMeshData(MeshData_t* pMeshData, int iMeshCount, std::vector<MeshData>& meshes, bool bIsAnim, AnimationData* pAnimData)
{
	for (int i = 0; i < iMeshCount; i++)
	{
		if (!bIsAnim)
		{
			pMeshData[i].pVertices = new Vertex_t[meshes[i].vertices.size()];
			pMeshData[i].uVerticeNum = meshes[i].vertices.size();
		}
		else
		{
			pMeshData[i].pSkinnedVertices = new SkinnedVertex_t[meshes[i].skinnedVertices.size()];
			pMeshData[i].uVerticeNum = meshes[i].skinnedVertices.size();
		}

		pMeshData[i].pIndices = new unsigned __int32[meshes[i].indices.size()];
		pMeshData[i].uIndicesNum = meshes[i].indices.size();

		if (!bIsAnim)
		{
			memcpy(pMeshData[i].pVertices, meshes[i].vertices.data(), sizeof(Vertex_t) * meshes[i].vertices.size());
		}
		else
		{
			memcpy(pMeshData[i].pSkinnedVertices, meshes[i].skinnedVertices.data(), sizeof(SkinnedVertex_t) * meshes[i].skinnedVertices.size());
		}

		memcpy(pMeshData[i].pIndices, meshes[i].indices.data(), sizeof(unsigned __int32) * meshes[i].indices.size());

		wcscpy_s(pMeshData[i].wcAlbedoTextureFilename, wstring(meshes[i].albedoTextureFilename.begin(), meshes[i].albedoTextureFilename.end()).c_str());
		wcscpy_s(pMeshData[i].wcEmissiveTextureFilename, wstring(meshes[i].emissiveTextureFilename.begin(), meshes[i].emissiveTextureFilename.end()).c_str());
		wcscpy_s(pMeshData[i].wcHeightTextureFilename, wstring(meshes[i].heightTextureFilename.begin(), meshes[i].heightTextureFilename.end()).c_str());
		wcscpy_s(pMeshData[i].wcNormalTextureFilename, wstring(meshes[i].normalTextureFilename.begin(), meshes[i].normalTextureFilename.end()).c_str());
		wcscpy_s(pMeshData[i].wcMetallicTextureFilename, wstring(meshes[i].metallicTextureFilename.begin(), meshes[i].metallicTextureFilename.end()).c_str());
		wcscpy_s(pMeshData[i].wcRoughnessTextureFilename, wstring(meshes[i].roughnessTextureFilename.begin(), meshes[i].roughnessTextureFilename.end()).c_str());
		wcscpy_s(pMeshData[i].wcAoTextureFilename, wstring(meshes[i].aoTextureFilename.begin(), meshes[i].aoTextureFilename.end()).c_str());
		wcscpy_s(pMeshData[i].wcOpacityTextureFilename, wstring(meshes[i].opacityTextureFilename.begin(), meshes[i].opacityTextureFilename.end()).c_str());

		ConvertFileExtensionToDDS(pMeshData[i].wcAlbedoTextureFilename);
		ConvertFileExtensionToDDS(pMeshData[i].wcEmissiveTextureFilename);
		ConvertFileExtensionToDDS(pMeshData[i].wcHeightTextureFilename);
		ConvertFileExtensionToDDS(pMeshData[i].wcNormalTextureFilename);
		ConvertFileExtensionToDDS(pMeshData[i].wcMetallicTextureFilename);
		ConvertFileExtensionToDDS(pMeshData[i].wcRoughnessTextureFilename);
		ConvertFileExtensionToDDS(pMeshData[i].wcAoTextureFilename);
		ConvertFileExtensionToDDS(pMeshData[i].wcOpacityTextureFilename);
	}

	NomalizeModelData(pMeshData, iMeshCount, 1.0f, bIsAnim, &pAnimData->defaultTransform);
}

void DestroyMeshData(MeshData_t* pMeshData, int iMeshCount)
{
	if (pMeshData)
	{
		for (int i = 0; i < iMeshCount; i++)
		{
			if (pMeshData[i].pVertices)
			{
				delete[] pMeshData[i].pVertices;
				pMeshData[i].pVertices = nullptr;
			}

			if (pMeshData[i].pSkinnedVertices)
			{
				delete[] pMeshData[i].pSkinnedVertices;
				pMeshData[i].pSkinnedVertices = nullptr;
			}

			if (pMeshData[i].pIndices)
			{
				delete[] pMeshData[i].pIndices;
				pMeshData[i].pIndices = nullptr;
			}
		}

		delete[] pMeshData;
	}
}

Model* CreateModel(MeshData_t* pMeshData, int iMeshDataNum)
{
	Model* pModel = new Model;
	pModel->Init(pMeshData, iMeshDataNum);
	g_vecAllModelList.push_back(pModel);
	return pModel;
}

SkinnedModel* CreateModel(MeshData_t* pMeshData, int iMeshDataNum, AnimationData* pAnimData)
{
	SkinnedModel* pModel = new SkinnedModel;
	pModel->Init(pMeshData, iMeshDataNum, pAnimData);
	g_vecAllModelList.push_back(pModel);
	return pModel;
}

ModelImporter* CreateModelImporter(string strOpenFilePath, string strOpenFileName, string strOutputFilrPath, bool bNormalRevert, bool bAnim)
{
	ModelImporter* importer = new ModelImporter;
	importer->Load(strOpenFilePath, strOpenFileName, strOutputFilrPath, bNormalRevert); // 텍스처 파일은 ../Asset/ 경로에 생성된다.
	g_vecModelImp.push_back(make_pair(importer, bAnim));
	return importer;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WndProc(HWND hWnd, AkU32 uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
	case WM_SIZE:
	{
		AkI32 width = LOWORD(lParam);  // Macro to get the low-order word.
		AkI32 height = HIWORD(lParam); // Macro to get the high-order word.
	}
	break;
	case WM_KEYDOWN:
	{
		cout << wParam << endl;

		g_bKeyDown[wParam] = true;

		if ('F' == wParam)
		{
			g_bFPV = !g_bFPV;
		}

		if (17 == wParam)
		{
			g_bCharacterDelete = true;
		}
	}
	break;
	case WM_KEYUP:
	{
		g_bKeyDown[wParam] = false;
	}
	break;
	case WM_MOUSEMOVE:
	{
		g_iMouseX = LOWORD(lParam);
		g_iMouseY = HIWORD(lParam);

		UpdateMouse(g_iMouseX, g_iMouseY);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		g_bLBtnDown = true;
	}
	break;
	case WM_LBUTTONUP:
	{
		g_bLBtnDown = false;
	}
	break;
	case WM_DESTROY:
	{
		::PostQuitMessage(999);
	}
	break;
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
