#pragma once

#include "Model.h"
#include "AnimationClip.h"

/*
==============
Skinned Model
==============
*/

struct Bone
{
	Vector3 vStart = Vector3(0.0f);
	Vector3 vEnd = Vector3(0.0f);

	struct Bone* pParent = nullptr;
	vector<struct Bone*> vecChild = {};
	vector<bool> vecChildDelete = {};
	
	string Name = "";
	int Id;
};

class SkinnedModel : public Model
{
public:
	SkinnedModel();
	~SkinnedModel();

	bool Init(MeshData_t* pMeshData, int iMeshCount, AnimationData* pAnimData);
	virtual void Update(const float fDT);
	virtual void Render();
	virtual void RenderShadow();

	void AttachWeaponLeftHand(Model* pWeapon);
	void AttachWeaponRightHand(Model* pWeapon);

private:
	void CleanUp();
	void CleanUpBoneTree(Bone* pTree);
	void UpdateGui();
	void UpdateArmatureTree(Bone* pTree);
	void PrintBoneTree(Bone* pTree, int iDepth);

private:
	AnimationData* _pAnimData = {};
	std::vector<Matrix> _vecBoneTransforms;
	Model* _pWeapon = nullptr;
	vector<pair<string, ILineObject*>> _vecLineObjs = {};
	vector<Matrix> _vecLineMat = {};
	Vector3 _vRightHandBonePos = Vector3(0.0f);
	Vector3 _vLeftHandBonePos = Vector3(0.0f);
	int _iLeftHandBoneID = 0;
	int _iRightHandBoneID = 0;
	bool _bUpdateFrame = false;
	int _iFrame = 0;
	vector<Bone*> _vecBoneTree = {};
	int _iSelectBoneID = -1;
	bool _bFlag = false;

public:
	bool UseLeftHand = false;
	bool UseRightHand = false;
};