#include "pch.h"
#include "UI.h"
#include "Application.h"
#include "GameInput.h"

/*
=========
UI
=========
*/

AkU32 UUI::sm_pInitRefCount;
ISpriteObject* UUI::sm_pCommonSpriteObj;

UUI::UUI()
{
}

UUI::~UUI()
{
	CleanUp();
}

AkBool UUI::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	if (sm_pInitRefCount)
	{
		return AK_TRUE;
	}

	_pRenderer = pApp->GetRenderer();

	sm_pCommonSpriteObj = _pRenderer->CreateSpriteObject();

	sm_pInitRefCount++;

	return AK_TRUE;
}

void UUI::GetRelativePosition(AkI32* pOutPosX, AkI32* pOutPosY)
{
	*pOutPosX = _iOffsetX;
	*pOutPosY = _iOffsetY;
}

void UUI::SetPosition(AkI32 iPosX, AkI32 iPosY)
{
	_iPosX = iPosX;
	_iPosY = iPosY;
}

void UUI::SetResolution(AkU32 uWidth, AkU32 uHeight)
{
	_uWidth = uWidth;
	_uHeight = uHeight;
}

void UUI::SetRelativePosition(AkI32 iPosX, AkI32 iPosY)
{
	_iOffsetX = iPosX;
	_iOffsetY = iPosY;
}

void UUI::SetScale(AkF32 fScaleX, AkF32 fScaleY)
{
	_fScaleX = fScaleX;
	_fScaleY = fScaleY;
}

void UUI::SetRect(const RECT* pRect)
{
	_tRect = *pRect; 
	_bUseRect = AK_TRUE;
}

void UUI::SetTexture(void* pTexHandle)
{
	_pTexHandle = pTexHandle;
}

void UUI::AddChildUI(UUI* pChild)
{
	tChildLinkUI.pData = pChild;

	LL_PushBack(&_pChildUIHead, &_pChildUITail, &tChildLinkUI);

	pChild->_pParentUI = this;

	SetDepth(_fDepth + AK_F32_EPSILON);
}

void UUI::SetDrawBackGround(AkBool bDrawBackGround)
{
	sm_pCommonSpriteObj->SetDrawBackground(bDrawBackGround);
}

void UUI::Update(const AkF32 fDeltaTime)
{
	MouseOnCheck();

	UpdateChildUI(fDeltaTime);
}

void UUI::Render()
{
	IRenderer* pRenderer = GetApp()->GetRenderer();

	pRenderer->RenderSpriteWithTex(sm_pCommonSpriteObj, _iPosX, _iPosY, _fScaleX, _fScaleY, _bUseRect ? &_tRect : nullptr, _fDepth, _pTexHandle);

	RenderChildUI();
}

void UUI::CleanUp()
{
	AkU32 uInitRefCount = --sm_pInitRefCount;

	CleanUpChildUI();

	if (!uInitRefCount)
	{
		if (sm_pCommonSpriteObj)
		{
			sm_pCommonSpriteObj->Release();
			sm_pCommonSpriteObj = nullptr;
		}
	}
}

void UUI::CleanUpChildUI()
{
	List_t* pCur = _pChildUIHead;
	List_t* pNext = nullptr;
	while (pCur != nullptr)
	{
		pNext = pCur->pNext;

		UUI* pChildUI = (UUI*)pCur->pData;

		pChildUI->CleanUpChildUI();

		if (pChildUI)
		{
			delete pChildUI;
			pChildUI = nullptr;
		}

		pCur = pNext;
	}
}

void UUI::MouseOnCheck()
{
	UGameInput* pGameInput = _pApp->GetGameInput();

	AkI32 iPosX = pGameInput->GetMouseX();
	AkI32 iPosY = pGameInput->GetMouseY();

	AkI32 iLeft = _iPosX;
	AkI32 iRight = iLeft + (AkI32)_uWidth;
	AkI32 iTop = _iPosY;
	AkI32 iBottom = iTop + (AkI32)_uHeight;

	if ((iLeft <= iPosX && iPosX <= iRight) && (iTop <= iPosY && iPosY <= iBottom))
	{
		_bIsMouseOn = AK_TRUE;
	}
	else
	{
		_bIsMouseOn = AK_FALSE;
	}
}

void UUI::UpdateChildUI(const AkF32 fDeltaTime)
{
	List_t* pCur = _pChildUIHead;
	while (pCur != nullptr)
	{
		AkI32 iOffsetX = 0;
		AkI32 iOffsetY = 0;

		UUI* pChildUI = (UUI*)pCur->pData;

		pChildUI->GetRelativePosition(&iOffsetX, &iOffsetY);
		pChildUI->SetPosition(_iPosX + iOffsetX, _iPosY + iOffsetY);
		pChildUI->Update(fDeltaTime);

		pCur = pCur->pNext;
	}
}

void UUI::RenderChildUI()
{
	List_t* pCur = _pChildUIHead;
	while (pCur != nullptr)
	{
		UUI* pChildUI = (UUI*)pCur->pData;

		pChildUI->Render();

		pCur = pCur->pNext;
	}
}
