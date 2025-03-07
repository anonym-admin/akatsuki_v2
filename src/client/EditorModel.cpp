#include "pch.h"
#include "EditorModel.h"
#include "Scene.h"
#include "Actor.h"
#include "SkinnedModel.h"
#include "Animator.h"
#include "Animation.h"
#include "Camera.h"
#include "Transform.h"


#include <string>

EditorModel::EditorModel()
{

}

EditorModel::~EditorModel()
{
	EndEditor();
}

AkBool EditorModel::BeginEditor()
{
	_pCamera = CreateCamera();
	_pTransform = CreateTransform();
	_pAnimation = CreateAnimation();

	return AK_TRUE;
}

AkBool EditorModel::EndEditor()
{
	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}
	if (_pTarget)
	{
		delete _pTarget;
		_pTarget = nullptr;
	}
	if (_pTransform)
	{
		delete _pTransform;
		_pTransform = nullptr;
	}
	if (_pAnimation)
	{
		delete _pAnimation;
		_pAnimation = nullptr;
	}

	return AK_TRUE;
}

void EditorModel::Update()
{
	EditorSetting();
	ModifiedAnimation();

	if (_pTarget)
	{
		if (_bIsSkinned)
		{
			if (_uClipNum)
			{
				((SkinnedModel*)_pTarget)->PlayAnimation(_wcCurClipName, AK_FALSE);
			}
		}
	}
}

void EditorModel::FinalUpdate()
{
	ImGui::Begin("Model Editor");
	ImGui::Checkbox("Blend Animation Mode", (bool*)&_bBlendMode);
	ImGui::Checkbox("FPV", (bool*)&_bFPV);
	ImGui::SliderInt("Frame", (int*)&_uAnimFrame, 0, 126);
	ImGui::Checkbox("Load", &_bLoad);
	ImGui::Checkbox("Save", &_bSave);
	ImGui::End();

	if (_bBlendMode)
		BlendMode();
	if (_bLoad)
		Load();
	if (_bSave)
		Save();

	// Update Transform.
	_pTransform->Update();

	// Update Model.
	if (_pTarget)
		_pTarget->UpdateWorldRow(&_pTransform->GetWorldTransform());

	// Update Camera.
	if (_bFPV)
		_pCamera->Update();
}

void EditorModel::Render()
{
	if(_pTarget)
		_pTarget->Render();
}

void EditorModel::RenderShadow()
{
	if(_pTarget)
		_pTarget->RenderShadow();
}

void EditorModel::EditorSetting()
{
	if (KEY_DOWN(KEY_INPUT_F))
		_bFPV = !_bFPV;
}

void EditorModel::ModifiedAnimation()
{
	if (_bBlendMode)
	{
	}
}

void EditorModel::BlendMode()
{
	AkBool bReset = AK_FALSE;
	ImGui::Begin("Blend Mode");

	char** ppClipNames = new char* [_countof(GAME_ANIM_PLAYER_ANIM_FILE_NAME)];
	for (AkU32 i = 0; i < _countof(GAME_ANIM_PLAYER_ANIM_FILE_NAME); i++)
	{
		size_t uResult = 0;
		ppClipNames[i] = new char[_MAX_PATH];
		wcstombs_s(&uResult, ppClipNames[i], _MAX_PATH, GAME_ANIM_PLAYER_ANIM_FILE_NAME[i], _MAX_PATH);
	}

	static AkU32 iCount = 0;
	static AkI32 iCurItem = 0;

	if(iCount < 2)
		if (ImGui::ListBox("Animation Clips", &iCurItem, ppClipNames, _countof(GAME_ANIM_PLAYER_ANIM_FILE_NAME)))
			iCount++;
	
	if (iCount == 1)
		_iCurAnimClip = iCurItem;
	else if(iCount == 2)
		_iNextAnimClip = iCurItem;

	if (iCount == 2)
	{
		ImGui::Text(ppClipNames[_iCurAnimClip]);
		ImGui::Text(ppClipNames[_iNextAnimClip]);
		ImGui::Checkbox("Reset", &bReset);
		ImGui::SliderFloat("Blending Time", (float*)&_fBlendTime, 0.0f, 100.0f);
	}

	if (bReset)
		iCount = 0;

	ImGui::End();

	for (AkU32 i = 0; i < _countof(GAME_ANIM_PLAYER_ANIM_FILE_NAME); i++)
	{
		delete[] ppClipNames[i];
	}
	delete[] ppClipNames;
}

void EditorModel::Load()
{
	// Load Model.
	{
		std::string filePath = "";
		std::string fileName = "";
		if (ImGui::Button("Load Model File"))
		{
			ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".md3d,.anim");
		}
		if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				AkBool bSkinnedFlag = AK_FALSE;
				filePath = ImGuiFileDialog::Instance()->GetFilePathName();
				fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

				std::string ext = GetFileExtension(fileName);
				if (ext == "anim" || ext == "md3d")
				{
					bSkinnedFlag = true;
				}

				filePath = GetFilePath(filePath);

				std::wstring wcFilePath;
				wcFilePath.assign(filePath.begin(), filePath.end());

				std::wstring wcFileName;
				wcFileName.assign(fileName.begin(), fileName.end());

				if (ext == "md3d")
				{
					_tContainer.pMeshData = GAssetManager->ReadFromFile(&_tContainer, &_tContainer.uMeshDataNum, wcFilePath.c_str(), wcFileName.c_str(), 1.0f, bSkinnedFlag);
					Vector3 vAlbedo = Vector3(1.0f);
					Vector3 vEmissive = Vector3(0.0f);
					_pTarget = CreateModel(&_tContainer, &vAlbedo, 0.0f, 1.0f, &vEmissive, bSkinnedFlag);
					_bIsSkinned = bSkinnedFlag;

					if (_tContainer.pMeshData)
					{
						for (AkU32 i = 0; i < _tContainer.uMeshDataNum; i++)
						{
							if (_tContainer.pMeshData[i].pVertices)
							{
								free(_tContainer.pMeshData[i].pVertices);
								_tContainer.pMeshData[i].pVertices = nullptr;
							}
							if (_tContainer.pMeshData[i].pSkinnedVertices)
							{
								free(_tContainer.pMeshData[i].pSkinnedVertices);
								_tContainer.pMeshData[i].pSkinnedVertices = nullptr;
							}
							if (_tContainer.pMeshData[i].pIndices)
							{
								free(_tContainer.pMeshData[i].pIndices);
								_tContainer.pMeshData[i].pIndices = nullptr;
							}
						}
						free(_tContainer.pMeshData);
						_tContainer.pMeshData = nullptr;
					}
				}
				if (ext == "anim")
				{
					if (_pTarget)
					{
						AnimationClip_t* pAnimClip = _pAnimation->ReadFromAnimationFile(wcFilePath.c_str(), wcFileName.c_str());
					}
				}
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}
}

void EditorModel::Save()
{
	std::string filePath = "";
	std::string fileName = "";
	if (ImGui::Button("Load Anim File"))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".anim");
	}
	// display
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			filePath = ImGuiFileDialog::Instance()->GetFilePathName();
			fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
		}
		ImGuiFileDialog::Instance()->Close();
	}
}