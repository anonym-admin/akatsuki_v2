#pragma once

#include "Editor.h"

class UCollider;
class UActor;
class UBackGroundUI;
class UBtnUI;
class UTextUI;

class UEditorModel : public UEditor
{
public:
	UEditorModel();
	~UEditorModel();

	virtual AkBool Initialize(UApplication* pApp) override;
	virtual void BeginEditor() override;
	virtual void EndEditor() override;
	virtual void Update(const AkF32 fDeltaTime) override;
	virtual void Render() override;

private:
	void CleanUp();

	void ProcessMousePicking();
	AkBool ProcessMousePickingWithSphere(UCollider* pCollider, Vector3* pHitPos, AkF32* pDist, AkF32* pRatio);
	UActor* PickClosets(Vector3* pHitPos, AkF32* pMinDist, AkF32* pRatio);

private:
	AkBool _bEnableCam = AK_FALSE;
	AkBool _bEnableEditorUI = AK_FALSE;

	UActor* _pPickingObj = nullptr;

	UBackGroundUI* _pBackGroundUI = nullptr;
	UBtnUI* _pBtnUI = nullptr;
	UTextUI* _pSystemTextUI = nullptr;
	UTextUI* _pEditorTextUI = nullptr;
};

