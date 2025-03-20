#pragma once

#include "Editor.h"

class ModelExporter;
class ModelImporter;

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

	void CreateModel(const std::wstring& wcBasePath, const std::wstring& wcFilename);
	void CreateClip(const std::wstring& wcPath, const std::wstring& wcClip);
	
	void BindAnimation();

	void UpdateFileDialog();
	void UpdateGizmo();
	void UpdateControl();

	void SetAnimation(const std::wstring& wcClip, AkF32 fSpeed, AkF32 fBlendTime);

private:
	Camera* _pCamera = nullptr;
	Vector3 _vCamPos = Vector3(0.0f, 0.0f, -2.0f);
	Vector3 _vCamYawPitchRoll = Vector3(0.0f, 0.0f, 0.0f);
	AkBool _bFPV = AK_TRUE;

	AkI32 _iExportType = -1;
	AkI32 _iImportType = -1;
	AkBool _bBindAnim = AK_FALSE;
	AkBool _bUseAnim = AK_FALSE;

	ModelExporter* _pExporter = nullptr;
	ModelImporter* _pImporter = nullptr;

	std::vector<Model*> _vecModel = {}; // 자료구조 다시 생각
	std::unordered_map<std::wstring, SkinnedModel*> _mapSkinnedModel = {};
	std::unordered_map<std::wstring, Animation*> _mapAnim = {};
	std::unordered_map<std::wstring, std::vector<std::wstring>> _mapClipName = {};

	// Bone Info.
	const Matrix* _pBoneOffsetMatrixList = nullptr;
	const AkI32* _pBoneHierarchyList = nullptr;
	AkU32 _uBoneNum = 0;
	Matrix _mDefaultMatrix = Matrix();

	std::wstring _CurModel = L"";
	std::wstring _CurClip = L"";

	AkF32 _fAnimSpeed = 1.0f;
	AkF32 _fBlendTime = 1.0f;
	AkF32 _fPrevAnimSpeed = 1.0f;
	AkF32 _fPrevBlendTime = 1.0f;
};

