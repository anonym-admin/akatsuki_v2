#include "pch.h"
#include "Sprite.h"

/*
===================
Sprite Effect
===================
*/

Sprite::Sprite(const wchar_t* wcTexFilename, const Vector2* pMaxFrame)
	: Particle(wcTexFilename)
{
	if (!Initialize(pMaxFrame))
	{
		__debugbreak();
	}
}

Sprite::~Sprite()
{
	CleanUp();
}

AkBool Sprite::Initialize(const Vector2* pMaxFrame)
{
	_vMaxFrame = *pMaxFrame;
	_uMaxFrameCount = (AkU32)(pMaxFrame->x * pMaxFrame->y);

	CreateParticles();

	return AK_TRUE;
}

void Sprite::Update()
{
	if (!_bIsPlay)
	{
		return;
	}

	_fTime += _fSpeed * DT;

	if (_fTime > 0.1f)
	{
		_uCurFrameCount++;

		_vCurFrame.x = (AkF32)(_uCurFrameCount % (AkU32)_vMaxFrame.x);
		_vCurFrame.y = (AkF32)(_uCurFrameCount / (AkU32)_vMaxFrame.x);

		_fTime = 0.0f;
	}

	if (_uCurFrameCount >= _uMaxFrameCount)
	{
		Stop();
	}

	_pTransform->Update();
}

void Sprite::UpdateEditor()
{
}

void Sprite::Render()
{
	GRenderer->RenderParticleSprite(_pParticle, _pDBHandle, &_vMaxFrame, &_vCurFrame);
}

void Sprite::CreateParticles()
{
	_uParticleCount = 1;

	_pVertices = new VertexSize_t;

	_pVertices->vSize = _vSize;

	_pParticle = GRenderer->CreateParticle();
	
	_pDBHandle = _pParticle->CreateParticleSprite(_pVertices);

	_pParticle->SetTexture(_pTexHandle);
}

void Sprite::Play(const Vector3* pPos)
{
	_bIsPlay = AK_TRUE;

	_fTime = 0.0f;
	_uCurFrameCount = 0;

	_pVertices->vPosition = Vector4(pPos->x, pPos->y, pPos->z, 1.0f);
	_pVertices->vSize = _vSize;

	GRenderer->UpdateDynamicDefaultBuffer(_pDBHandle, _pVertices);
}

void Sprite::CleanUp()
{
	if (_pVertices)
	{
		delete _pVertices;
		_pVertices = nullptr;
	}
}
