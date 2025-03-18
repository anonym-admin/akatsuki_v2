#include "pch.h"
#include "Particle.h"

/*
===========
Particle
===========
*/

Particle::Particle(const wchar_t* wcFilename)
{
	if (!Initialize(wcFilename))
	{
		__debugbreak();
	}
}

Particle::~Particle()
{
	CleanUp();
}

AkBool Particle::Initialize(const wchar_t* wcFilename)
{
	_pTransform = new Transform;

	_pTexHandle = GRenderer->CreateTextureFromFile(wcFilename, AK_TRUE);

	return AK_TRUE;
}

void Particle::Render()
{
}

void Particle::Play(const Vector3* pPos)
{
	_bIsPlay = AK_TRUE;
	_pTransform->SetPosition(pPos);
}

void Particle::Stop()
{
}

void Particle::Pause()
{
	_bIsPlay = AK_FALSE;
}

void Particle::Resume()
{
	_bIsPlay = AK_TRUE;
}

void Particle::CleanUp()
{
	if (_pDBHandle)
	{
		_pParticle->DestroyBasicParticleBuffer(_pDBHandle);
		_pDBHandle = nullptr;
	}
	if (_pParticle)
	{
		_pParticle->Release();
		_pParticle = nullptr;
	}
	if (_pTexHandle)
	{
		GRenderer->DestroyTexture(_pTexHandle);
		_pTexHandle = nullptr;
	}
	if (_pTransform)
	{
		delete _pTransform;
		_pTransform = nullptr;
	}
}
