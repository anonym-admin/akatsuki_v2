#pragma once

#include "Editor.h"

class ModelExporter;
class ModelImporter;
class Collider;

class EditorModel : public Editor
{
public:
	EditorModel();
	~EditorModel();

	AkBool Initialize();
	virtual AkBool BeginEditor() override;
	virtual AkBool EndEditor() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void RenderShadow() override;
	virtual void RenderGUI() override;

private:
	void CleanUp();

	virtual void Load(const std::wstring& wcFilePath) override {};
	virtual void Save(const std::wstring& wcFilePath) override {};

	void ExportMesh(const std::wstring& wcName, const std::wstring& wcExt);
	void ExportAnimation(const std::wstring& wcName, const std::wstring& wcClip);
	void ModifyAnimation(const std::wstring& wcName, const std::wstring& wcClip);

	void CreateModel(const std::wstring& wcBasePath, const std::wstring& wcFilename);
	void CreateClip(const std::wstring& wcPath, const std::wstring& wcClip);
	void CreateCollider(COLLIDER_TYPE eTyep);
	
	void BindAnimation();
	void AttachBone();

	void UpdateFileDialog();
	void UpdateGizmo();
	void UpdateWeapon();
	void UpdateControl();

	void SetAnimation(const std::wstring& wcClip, AkF32 fSpeed, AkF32 fBlendTime);

private:
	Camera* _pCamera = nullptr;
	Vector3 _vCamPos = Vector3(0.0f, 1.5f, -2.0f);
	Vector3 _vCamYawPitchRoll = Vector3(0.0f, 0.0f, 0.0f);
	AkBool _bFPV = AK_TRUE;

	Model* _pGround = nullptr;

	AkI32 _iExportType = -1;
	AkI32 _iModifyType = -1;
	AkI32 _iImportType = -1;
	AkI32 _iCurSelectedBoneID = -1;
	AkI32 _iSelectColliderTpye = -1;
	AkI32 _iPrevSelectedBoneID = 0;
	AkBool _bBindAnim = AK_FALSE;
	AkBool _bPlayAnim = AK_FALSE;
	AkBool _bUseAnim = AK_FALSE;
	AkBool _bAttachBone = AK_FALSE;
	AkBool _bModifyWeaponTransform = AK_FALSE;
	AkBool _bAttachColliderToCharacter = AK_FALSE;
	AkBool _bRenderBone = AK_FALSE;
	AkBool _bRenderCharacter = AK_TRUE;

	ModelExporter* _pExporter = nullptr;
	ModelImporter* _pImporter = nullptr;

	std::vector<Model*> _vecModel = {}; // 자료구조 다시 생각
	std::unordered_map<std::wstring, SkinnedModel*> _mapSkinnedModel = {};
	std::unordered_map<std::wstring, Animation*> _mapAnim = {};
	std::unordered_map<std::wstring, AnimationClip_t*> _mapClip = {};
	std::unordered_map<std::wstring, std::vector<std::wstring>> _mapClipName = {};

	// Bone Info.
	const Matrix* _pBoneOffsetMatrixList = nullptr;
	const AkI32* _pBoneHierarchyList = nullptr;
	char** _ppBoneName = nullptr;
	AkU32 _uBoneNum = 0;
	Matrix _mDefaultMatrix = Matrix();

	std::wstring _CurCharacter = L"";
	std::wstring _CurClip = L"";
	std::wstring _PickModel = L"";

	AkF32 _fAnimSpeed = 1.0f;
	AkF32 _fBlendTime = 1.0f;
	AkF32 _fPrevAnimSpeed = 1.0f;
	AkF32 _fPrevBlendTime = 1.0f;

	BoneAnimation_t* _pCurBoneAnimation = nullptr;
	BoneAnimation_t* _pAttachBoneAnimation = nullptr;
	BoneAnimation_t* _pCombineBoneAnimation = nullptr;
	AnimationClip_t* _pCombineAnimationClip = nullptr;

	Vector2 _vCurClipBoneID = Vector2(-1.0f);
	Vector2 _vAttachClipBoneID = Vector2(-1.0f);

	Matrix _mAttachMatrix = Matrix();

	// Collider.
	// 삭제 기능 미구현.
	Collider* _pCollider = nullptr;

	// For Shadow.
	Vector3 _vRadiance = Vector3(0.5f);
	Vector3 _vLightDir = Vector3(-20.0f, 50.0f, 20.0f);

	// Debug Shadow.
	ISprite* _pDebugShadowMap = nullptr;
};

