#pragma once

#include "Editor.h"

class Collider;
class Actor;
class UBackGroundUI;
class UBtnUI;
class TextUI;

class UEditorModel : public UEditor
{
public:
	UEditorModel();
	~UEditorModel();

	virtual AkBool Initialize(Application* pApp) override;
	virtual void BeginEditor() override;
	virtual void EndEditor() override;
	virtual void Update(const AkF32 fDeltaTime) override;
	virtual void Render() override;

private:
	void CleanUp();

	void ProcessMousePicking();
	AkBool ProcessMousePickingWithSphere(Collider* pCollider, Vector3* pHitPos, AkF32* pDist, AkF32* pRatio);
	Actor* PickClosets(Vector3* pHitPos, AkF32* pMinDist, AkF32* pRatio);

private:
	AkBool _bEnableCam = AK_FALSE;
	AkBool _bEnableEditorUI = AK_FALSE;

	Actor* _pPickingObj = nullptr;

	UBackGroundUI* _pBackGroundUI = nullptr;
	UBtnUI* _pBtnUI = nullptr;
	TextUI* _pSystemTextUI = nullptr;
	TextUI* _pEditorTextUI = nullptr;
};

