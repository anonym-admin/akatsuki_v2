#include "SkinnedModel.h"
#include <imgui.h>
#include <ImGuizmo.h>

extern IRenderer* g_pRenderer;
extern bool g_bEnableAnim;
extern bool g_bAttachWeaponLeftHand;
extern int g_iMouseX;
extern int g_iMouseY;
extern float g_fNdcX;
extern float g_fNdcY;

extern Matrix g_mViewMat;
extern Matrix g_mProjMat;

static bool useSnap(false);
static float snap[3] = { 1.f, 1.f, 1.f };

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

/*
==============
Skinned Model
==============
*/

SkinnedModel::SkinnedModel()
{
}

SkinnedModel::~SkinnedModel()
{
	CleanUp();
}


bool SkinnedModel::Init(MeshData_t* pMeshData, int iMeshCount, AnimationData* pAnimData)
{
	_pMeshObj = g_pRenderer->CreateSkinnedMeshObject();

	_pAnimData = pAnimData;

	Vector3 vAlbedo = Vector3(1.0f);
	AkF32 fMetallic = 0.0f;
	AkF32 fRoughness = 1.0f;
	Vector3 vEmission = Vector3(0.0f);
	_pMeshObj->CreateMeshBuffers(pMeshData, iMeshCount);
	_pMeshObj->UpdateMaterialBuffers(&vAlbedo, fMetallic, fRoughness, &vEmission);

	// _vecBoneTransforms.resize(tAnimData.boneTransforms.size());
	_vecBoneTransforms.resize(96); // Max bone transform count in renderer => 96 
	_vecLineMat.resize(pAnimData->boneTransforms.size());

	Vector3 vBoneInitPos = Vector3(0.0f);

	// Create the line object for bone rendering.
	bool bLeftHandFind = false;
	bool bRightHandFind = false;
	for (int iBoneId = 0; iBoneId < pAnimData->boneTransforms.size(); iBoneId++)
	{
		int iParentBoneId = (pAnimData->boneParents[iBoneId] == -1) ? 0 : pAnimData->boneParents[iBoneId];
		Vector3 vStart = Vector3::Transform(vBoneInitPos, pAnimData->offsetMatrices[iBoneId].Invert() * pAnimData->defaultTransform);
		Vector3 vEnd = Vector3::Transform(vBoneInitPos, pAnimData->offsetMatrices[iParentBoneId].Invert() * pAnimData->defaultTransform);
		Vector3 vColor = Vector3(0.0f, 1.0f, 0.0f);

		LineVertex_t tStart = { vStart, vColor };
		LineVertex_t tEnd = { vEnd, vColor };
		ILineObject* pLineObj = g_pRenderer->CreateLineObject();
		pLineObj->CreateLineBuffers(&tStart, &tEnd);

		cout << pAnimData->boneIdToName[iBoneId] << endl;
		if (pAnimData->boneIdToName[iBoneId].find("LeftHand") != string::npos && !bLeftHandFind)
		{
			// 무기 장착 시 위치 조정 필요.
			_vLeftHandBonePos = vStart;
			_iLeftHandBoneID = iBoneId;
			bLeftHandFind = true;
		}
		if (pAnimData->boneIdToName[iBoneId].find("RightHand") != string::npos && !bRightHandFind)
		{
			// 무기 장착 시 위치 조정 필요.
			_vRightHandBonePos = vStart;
			_iRightHandBoneID = iBoneId;
			bRightHandFind = true;
		}

		_vecLineObjs.push_back(make_pair(pAnimData->boneIdToName[iParentBoneId], pLineObj));
	}

	// Init Bone tree.
	Bone* pBone = nullptr;
	for (int i = 0; i < pAnimData->boneTransforms.size(); i++)
	{
		pBone = new Bone;
		pBone->Name = pAnimData->boneIdToName[i];
		pBone->Id = i;
		_vecBoneTree.push_back(pBone);
		
	}

	for (int i = pAnimData->boneTransforms.size() - 1; i >= 0; i--)
	{
		int iId = _vecBoneTree[i]->Id;
		int iParentId = pAnimData->boneParents[iId];
		if (iParentId == -1)
		{
			_vecBoneTree[i]->pParent = nullptr;
		}
		else
		{
			Bone* pParentBone = _vecBoneTree[iParentId];
			_vecBoneTree[i]->pParent = pParentBone;
			_vecBoneTree[i]->pParent->vecChild.push_back(_vecBoneTree[i]);
			_vecBoneTree[i]->pParent->vecChildDelete.push_back(false);
		}
	}

	// Debug.
	// Bone* pCur = _vecBoneTree[0];
	// PrintBoneTree(pCur, 0);

	BndBox.Center = Vector3(0.0f);
	BndBox.Extents = Vector3(0.5f, 0.5f, 0.5f);

	return true;
}

void SkinnedModel::Update(const float fDT)
{
	Model::Update(fDT);

	_pAnimData->Update(0, _iFrame);

	// T-pose.
	for (int i = 0; i < _pAnimData->boneTransforms.size(); i++)
	{
		_vecBoneTransforms[i] = Matrix();
	}

	// Update Anim Transform.
	if (g_bEnableAnim)
	{
		for (int i = 0; i < _pAnimData->boneTransforms.size(); i++)
		{
			_vecBoneTransforms[i] = _pAnimData->Get(0, i, _iFrame).Transpose();
		}

		if (_pWeapon)
		{
			if (UseRightHand)
			{
				Matrix mRightHandBoneTransform =
					_pAnimData->defaultTransform.Invert() *
					_pAnimData->offsetMatrices[_iRightHandBoneID] *
					_pAnimData->boneTransforms[_iRightHandBoneID] *
					_pAnimData->defaultTransform;

				_pWeapon->SetRightHandBoneTransform(mRightHandBoneTransform);
			}
			else
			{
				_pWeapon->RightHand = false;
			}
			if(UseLeftHand)
			{
				Matrix mLeftHandBoneTransform =
					_pAnimData->defaultTransform.Invert() *
					_pAnimData->offsetMatrices[_iLeftHandBoneID] *
					_pAnimData->boneTransforms[_iLeftHandBoneID] *
					_pAnimData->defaultTransform;

				_pWeapon->SetLeftHandBoneTransform(mLeftHandBoneTransform);
			}
			else
			{
				_pWeapon->LeftHand = false;
			}
		}
	}

	UpdateGui();

	// _iFrame++;
}

void SkinnedModel::Render()
{
	Matrix& mMatrix = GetWorldRow();

	g_pRenderer->RenderSkinnedMeshObject(_pMeshObj, &mMatrix, _vecBoneTransforms.data());

	int i = 0;
	for (const auto& e : _vecLineObjs)
	{
		int boneId = _pAnimData->boneNameToId[e.first];
		_vecLineMat[i] = _pAnimData->Get(0, boneId, 0);
		g_pRenderer->RenderLineObject(e.second, &_vecLineMat[i]);
		i++;
	}
}

void SkinnedModel::RenderShadow()
{
	Matrix& mMatrix = GetWorldRow();

	g_pRenderer->RenderShadowOfSkinnedMeshObject(_pMeshObj, &mMatrix, _vecBoneTransforms.data());
}

void SkinnedModel::AttachWeaponLeftHand(Model* pWeapon)
{
	_pWeapon = pWeapon;
	_pWeapon->pOwner = this;
	UseLeftHand = true;

	//pWeapon->SetPosition(_vLeftHandBonePos);
	Matrix& mWorldRow = pWeapon->GetWorldRow();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow *= Matrix::CreateTranslation(_vLeftHandBonePos);
}

void SkinnedModel::AttachWeaponRightHand(Model* pWeapon)
{
	_pWeapon = pWeapon;
	_pWeapon->pOwner = this;
	UseRightHand = true;

	//pWeapon->SetPosition(_vRightHandBonePos);
	Matrix& mWorldRow = pWeapon->GetWorldRow();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow *= Matrix::CreateTranslation(_vRightHandBonePos);
}

void SkinnedModel::CleanUp()
{
	CleanUpBoneTree(_vecBoneTree[0]);

	for (auto& e : _vecLineObjs)
	{
		if (e.second)
		{
			e.second->Release();
			e.second = nullptr;
		}
	}
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}
}

void SkinnedModel::CleanUpBoneTree(Bone* pTree)
{
	for (int i = 0; i < _vecBoneTree.size(); i++)
	{
		if (_vecBoneTree[i])
		{
			delete _vecBoneTree[i];
			_vecBoneTree[i] = nullptr;
		}
	}
	_vecBoneTree.clear();
}

void SkinnedModel::UpdateGui()
{
	ImGui::Begin("Anim control");
	ImGui::Checkbox("Use Anim", &g_bEnableAnim);
	ImGui::Checkbox("Update Frame", &_bUpdateFrame);
	ImGui::SliderInt("Frame Count", &_iFrame, 0, 230);

	if (ImGui::TreeNode("Armature"))
	{
		UpdateArmatureTree(_vecBoneTree[0]);

		ImGui::TreePop();
	}

	if (_iSelectBoneID >= 0)
	{
		string strInfoName = _pAnimData->boneIdToName[_iSelectBoneID] + " Clip Info";
		auto& keys = _pAnimData->clips[0].keys[_iSelectBoneID];
		int iFrame = _iFrame % keys.size();
		if (keys.empty())
		{
			ImGui::Text("Empty Keys Data");
		}
		else
		{
			ImGui::Text(strInfoName.c_str());
			ImGui::SliderFloat3("Pos", &keys[iFrame].pos.x, -100.0f, 100.0f);
			ImGui::SliderFloat4("Rot", &keys[iFrame].rot.x, -1.0f, 1.0f);
			ImGui::SliderFloat3("Scale", &keys[iFrame].scale.x, -1.0f, 1.0f);
		}
	}

	ImGui::Checkbox("Other KeyFrame Update", &_bFlag);
	if (_bFlag)
	{
		auto& keys = _pAnimData->clips[0].keys[_iSelectBoneID];
		for (auto& e : keys)
		{
			e = keys[0];
		}
	}

	ImGui::End();
}

void SkinnedModel::UpdateArmatureTree(Bone* pTree)
{
	if (!pTree)
		return;

	// 알고리즘 수정 필요.
	if (ImGui::TreeNode(pTree->Name.c_str()))
	{
		_iSelectBoneID = pTree->Id;

		for (int i = 0; i < pTree->vecChild.size(); i++)
			UpdateArmatureTree(pTree->vecChild[i]);

		ImGui::TreePop();
	}
}

void SkinnedModel::PrintBoneTree(Bone* pTree, int iDepth)
{
	if (!pTree)
		return;

	for (int i = 0; i < iDepth; i++)
		cout << " ";

	cout << pTree->Name << " " << pTree->Id << endl;

	for (int i = 0; i < pTree->vecChild.size(); i++)
		PrintBoneTree(pTree->vecChild[i], iDepth + 1);
}
