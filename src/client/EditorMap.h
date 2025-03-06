#pragma once

#include "Editor.h"

class UTextUI;

class UEditorMap : public UEditor
{
public:
	UEditorMap();
	virtual ~UEditorMap();

	virtual AkBool Initialize(UApplication* pApp) override;
	virtual void BeginEditor() override;
	virtual void EndEditor() override;
	virtual void Update(const AkF32 fDeltaTime) override;
	virtual void Render() override;

private:
	void CleanUp();

	void UpdateMapObject();
	void UpdateText();

	void CreateBox();

private:
	AkBool _bEnableCam = AK_FALSE;
	Vector3 _vLeftTopPos = Vector3(0.0f);
	Vector3 _vRightBottomPos = Vector3(0.0f);

	UTextUI* _pSystemTextUI = nullptr;
};

