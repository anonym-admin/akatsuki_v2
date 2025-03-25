#pragma once

#include "Editor.h"

/*
=============
Model Editor
=============
*/

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
	virtual void Save(const std::wstring& wcFilePath) override; // Save .act file.

	void ExportMesh(const std::wstring& wcName, const std::wstring& wcExt);
	void ExportAnimation(const std::wstring& wcName, const std::wstring& wcClip);
	void ModifyAnimation(const std::wstring& wcName, const std::wstring& wcClip);

	void CreateModel(const std::wstring& wcBasePath, const std::wstring& wcFilename);
	void CreateClip(const std::wstring& wcPath, const std::wstring& wcClip);
	void CreateCollider(COLLIDER_TYPE eTyep);
	void CreateMeshFile(const std::wstring& wcName);
	void LoadTextures(const std::wstring& wcBasePath, const std::wstring& wcFilename);
	
	void BindAnimation();
	void AttachBone();

	void UpdateFileDialog();
	void UpdateGizmo();
	void UpdateWeapon();
	void UpdateControl();

	void SetAnimation(const std::wstring& wcClip, AkF32 fSpeed, AkF32 fBlendTime);

	void SaveMesh(const std::wstring& wcName, MeshData_t* pMeshData, AkU32 uMeshDataNum, void* pPickID);

	void DeleteProcess();

private:
	Camera* _pCamera = nullptr;
	Vector3 _vCamPos = Vector3(0.0f, 1.5f, -2.0f);
	Vector3 _vCamYawPitchRoll = Vector3(0.0f, 0.0f, 0.0f);
	AkBool _bFPV = AK_TRUE;

	Model* _pGround = nullptr;

	AkI32 _iExportType = -1;
	AkI32 _iModifyType = -1;
	AkI32 _iImportType = -1;
	AkI32 _iTextureType = -1;
	AkI32 _iGeometryType = -1;
	AkI32 _iSaveActorDataType = -1;
	AkI32 _iCurSelectedBoneID = -1;
	AkI32 _iSelectColliderTpye = -1;
	AkI32 _iPrevSelectedBoneID = 0;
	AkBool _bBindAnim = AK_FALSE;
	AkBool _bPlayAnim = AK_FALSE;
	AkBool _bUseAnim = AK_FALSE;
	AkBool _bAttachBone = AK_FALSE;
	AkBool _bModifyWeaponTransform = AK_FALSE;
	AkBool _bRenderBone = AK_FALSE;
	AkBool _bRenderCharacter = AK_TRUE;
	AkBool _bMatchingAABB = AK_FALSE;

	ModelExporter* _pExporter = nullptr;
	ModelImporter* _pImporter = nullptr;

	std::vector<Model*> _vecModel = {};
	std::unordered_map<std::wstring, Model*> _mapBasicModel = {};
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

	Matrix _mAttachMatrix = Matrix();

	// Textures.
	std::unordered_map<void*, std::vector<std::wstring>> _mapTextures = {};
	AkF32 _fGeoSphereRadius = 1.0f;
	AkU32 _uGeoSphereSlice = 32;
	AkU32 _uGeoSphereStack = 32;

	// Collider.
	std::vector<Collider*> _vecColliders = {};
	std::unordered_map<std::wstring, std::vector<Collider*>> _mapColliders = {};
	std::unordered_map<std::wstring, std::pair<Vector3, Vector3>> _mapAABB = {};

	// For Shadow.
	Vector3 _vRadiance = Vector3(0.5f);
	Vector3 _vLightDir = Vector3(-20.0f, 50.0f, 20.0f);
};

