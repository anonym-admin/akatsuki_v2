#include "pch.h"
#include "SoundManager.h"
#include "Sound.h"

/*
===================
Sound Manager
===================
*/

SoundManager::SoundManager()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

SoundManager::~SoundManager()
{
	CleanUp();
}

AkBool SoundManager::Initialize()
{
	FMOD_RESULT tRet;

	tRet = FMOD::System_Create(&_pSystem);
	if (tRet != FMOD_OK)
	{
		return AK_FALSE;
	}

	tRet = _pSystem->init(32, FMOD_INIT_NORMAL, nullptr);
	if (tRet != FMOD_OK)
	{
		return AK_FALSE;
	}

	return AK_TRUE;
}

void SoundManager::Update()
{
	_pSystem->update();
}

void SoundManager::Render()
{
}

Sound* SoundManager::LoadSound(const char* pFilename)
{
	FMOD_RESULT tRet;

	Sound* pSound = new Sound;
	if (!pSound->Initialize(_pSystem))
	{
		__debugbreak();
		delete pSound;
		pSound = nullptr;
		return pSound;
	}

	FMOD::Sound** ppSysSound = pSound->GetSoundAddressOf();

	tRet = _pSystem->createSound(pFilename, FMOD_DEFAULT, 0, ppSysSound);
	if (tRet != FMOD_OK)
	{
		(*ppSysSound)->release();
		(*ppSysSound) = nullptr;
		return pSound;
	}

	_pSound = pSound;

	return pSound;
}

void SoundManager::CleanUp()
{
	if (_pSound)
	{
		delete _pSound;
		_pSound = nullptr;
	}
	if (_pSystem)
	{
		_pSystem->close();
		_pSystem->release();
		_pSystem = nullptr;
	}
}