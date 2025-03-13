#pragma once

/*
=============
Button UI
=============
*/

class UIImage;

class UIButton
{
public:
	UIButton(const wchar_t* wcName, const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY);
	virtual ~UIButton();

	AkBool Initialize(const wchar_t* wcName, const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY);
	virtual void Update();
	virtual void FinalUpdate();
	virtual void Render();

	Collider* CreateCollider();
	void CreateTexture(const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY);
	void DestroyCollider();
	void DestroyTexture();

private:
	void CleanUp();

	void UpdateState();

private:
	UIImage* _pUIImage = nullptr;
	Collider* _pCollider = nullptr;

public:
	const wchar_t* Name = nullptr;
};

