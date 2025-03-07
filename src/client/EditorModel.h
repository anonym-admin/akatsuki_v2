#pragma once

#include "Editor.h"

class EditorModel : public Editor
{
public:
	EditorModel();
	~EditorModel();

	virtual AkBool BeginEditor() override;
	virtual AkBool EndEditor() override;
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

private:
	void EditorSetting();
	void ModifiedAnimation();

	void BlendMode();
	virtual void Load() override;
	virtual void Save() override;

private:
	Vector3 _vSceneCamPos = Vector3(0.0f);
	AkI32 _iCurAnimClip = 0;
	AkI32 _iNextAnimClip = 0;
	AkU32 _uAnimFrame = 0;
	AkBool _bFPV = AK_FALSE;
	AkBool _bLoad = AK_FALSE;
	AkBool _bSave = AK_FALSE;
	AkBool _bIsSkinned = AK_FALSE;

	AkBool _bBlendMode = AK_FALSE;
	AkF32 _fBlendTime = 0.0f;

	AkU32 _uClipNum = 0;
	const wchar_t* _wcCurClipName = nullptr;

	AssetMeshDataContainer_t _tContainer = {};
};

