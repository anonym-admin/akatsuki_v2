#pragma once

#include "UI.h"

/*
===========
Panel UI
===========
*/

class UPanelUI : public UUI 
{
public:
	UPanelUI();
	virtual ~UPanelUI();

	AkBool Initialize(UApplication* pApp, const wchar_t* wcTexFileName, AkU32 uPosX, AkU32 uPosY, AkU32 uTexWidth, AkU32 uTexHeight);

	virtual void MouseOn() override;
	virtual void MouseLBtnDown() override;
	virtual void MouseLBtnUp() override;
	virtual void MouseLBtnClick() override;

	virtual void SetDrawBackGround(AkBool bDrawBackGround) override;

	virtual void Render() override;

	UPanelUI* Clone();

private:
	virtual void CleanUp() override;

private:
	AkBool _bClone = AK_FALSE;
	IRenderer* _pRenderer = nullptr;
	ISpriteObject* _pTexSpriteObj = nullptr;
	void* _pTexHandle = nullptr;
};