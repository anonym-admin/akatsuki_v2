#pragma once

#include "PanelUI.h"
#include "Application.h"

/*
===============
Button UI
===============
*/

typedef void(UApplication::*MFP_ExitGame)(void);

class UBtnUI : public UPanelUI
{
public:
	UBtnUI();
	~UBtnUI();

	AkBool Initialize(UApplication* pApp, const wchar_t* wcTexFileName, AkU32 uPosX, AkU32 uPosY, AkU32 uTexWidth, AkU32 uTexHeight);
	void SetClickFunc(MFP_ExitGame pFunc) { _pExitGameFunc = pFunc; }

	virtual void Render() override;

	virtual void MouseOn() override;
	virtual void MouseLBtnDown() override;
	virtual void MouseLBtnUp() override;
	virtual void MouseLBtnClick() override;

private:
	virtual void CleanUp() override;

private:
	UApplication* _pApp = nullptr;
	MFP_ExitGame _pExitGameFunc = nullptr;
};
