#include "pch.h"
#include "UIButton.h"
#include "UIImage.h"

/*
=============
Button UI
=============
*/

UIButton::UIButton(const wchar_t* wcName, const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY)
{
    if (!Initialize(wcName, wcTextureFilename, iTexPosX, iTexPosY, iTexOffsetX, iTexOffsetY, iRenderPosX, iRenderPosY))
    {
        __debugbreak();
    }
}

UIButton::~UIButton()
{
}

AkBool UIButton::Initialize(const wchar_t* wcName, const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY)
{
    Name = wcName;

    CreateTexture(wcTextureFilename, iTexPosX, iTexPosY, iTexOffsetX, iTexOffsetY, iRenderPosX, iRenderPosY);
    _pCollider = CreateCollider();

    return AkBool();
}

void UIButton::Update()
{
}

void UIButton::FinalUpdate()
{
}

void UIButton::Render()
{
    _pUIImage->Render();
}

Collider* UIButton::CreateCollider()
{
    AkI32 iLeft = 0;
    AkI32 iTop = 0;
    AkI32 iRight = 0;
    AkI32 iBottom = 0;

    if (_pUIImage)
    {
        _pUIImage->Rect(&iLeft, &iTop, &iRight, &iBottom);
    }

    Collider* pCollider = new SquareCollider(iLeft, iTop, iRight, iBottom);
    return pCollider;
}

void UIButton::CreateTexture(const wchar_t* wcTextureFilename, AkI32 iTexPosX, AkI32 iTexPosY, AkI32 iTexOffsetX, AkI32 iTexOffsetY, AkI32 iRenderPosX, AkI32 iRenderPosY)
{
    _pUIImage = new UIImage(wcTextureFilename, iTexPosX, iTexPosY, iTexOffsetX, iTexOffsetY, iRenderPosX, iRenderPosY);
}

void UIButton::UpdateState()
{
}

void UIButton::DestroyCollider()
{
    if (_pCollider)
    {
        delete _pCollider;
        _pCollider = nullptr;
    }
}

void UIButton::DestroyTexture()
{
    if (_pUIImage)
    {
        delete _pUIImage;
        _pUIImage = nullptr;
    }
}

void UIButton::CleanUp()
{
    DestroyCollider();
    DestroyTexture();
}

