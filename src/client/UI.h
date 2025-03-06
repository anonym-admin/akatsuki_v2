#pragma once

/*
=========
UI
=========
*/

class UApplication;

class UUI
{
public:
	UUI();
	virtual ~UUI() = 0;

	AkBool Initialize(UApplication* pApp);
	UApplication* GetApp() { return _pApp; }
	ISpriteObject* GetCommonSpriteObj() { return sm_pCommonSpriteObj; }
	void GetRelativePosition(AkI32* pOutPosX, AkI32* pOutPosY);
	List_t* GetChildUIListHead() { return _pChildUIHead; }
	AkBool IsMouseOn() { return _bIsMouseOn; }
	AkBool IsMouseLBtnDown() { return _bIsLBtnDown; }
	void SetPosition(AkI32 iPosX, AkI32 iPosY);
	void SetResolution(AkU32 uWidth, AkU32 uHeight);
	void SetDepth(AkF32 fDepth) { _fDepth = fDepth; }
	void SetRelativePosition(AkI32 iPosX, AkI32 iPosY);
	void SetScale(AkF32 fScaleX, AkF32 fScaleY);
	void SetRect(const RECT* pRect); // For Texture Sampling.
	void SetTexture(void* pTexHandle);
	void AddChildUI(UUI* pChild);
	void SetParentUI(UUI* pParent) { _pParentUI = pParent; }
	
	virtual void SetDrawBackGround(AkBool bDrawBackGround);

	virtual void Update(const AkF32 fDeltaTime);
	virtual void Render();

	virtual void MouseOn() = 0;
	virtual void MouseLBtnDown() = 0;
	virtual void MouseLBtnUp() = 0;
	virtual void MouseLBtnClick() = 0;

	List_t tLinkUI = {};
	List_t tChildLinkUI = {};

protected:
	void MouseOnCheck();
	void UpdateChildUI(const AkF32 fDeltaTime);
	void RenderChildUI();

private:
	virtual void CleanUp() = 0;

	void CleanUpChildUI();

private:
	static AkU32 sm_pInitRefCount;
	IRenderer* _pRenderer = nullptr;
	UApplication* _pApp = nullptr;
	void* _pTexHandle = nullptr;
	AkU8* _pTexImage = nullptr;
	List_t* _pChildUIHead = nullptr;
	List_t* _pChildUITail = nullptr;
	UUI* _pParentUI = nullptr;

protected:
	static ISpriteObject* sm_pCommonSpriteObj;
	AkI32 _iPosX = 0;
	AkI32 _iPosY = 0;
	AkU32 _uWidth = 0;
	AkU32 _uHeight = 0;
	AkI32 _iOffsetX = 0;
	AkI32 _iOffsetY = 0;;
	AkF32 _fScaleX = 1.0f;
	AkF32 _fScaleY = 1.0f;
	RECT _tRect = {};
	AkF32 _fDepth = 0.0f;
	AkBool _bUseRect = AK_FALSE;
	AkBool _bIsMouseOn = AK_FALSE;
	AkBool _bIsLBtnDown = AK_FALSE;
};

