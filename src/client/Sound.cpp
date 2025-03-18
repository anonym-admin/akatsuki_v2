#include "pch.h"
#include "Sound.h"

/*
===========
Sound
===========
*/

Sound::Sound()
{
}

Sound::~Sound()
{
	CleanUp();
}

AkBool Sound::Initialize(FMOD::System* pSys)
{
	_pSystem = pSys;

	return AK_TRUE;
}

void Sound::Update()
{
}

void Sound::Render()
{
}

AkBool Sound::Play(AkBool bLoop)
{
	if (_pChannel)
	{
		_pChannel->isPlaying((bool*)&_bPlaying);
	}
	if (!_bPlaying)
	{
		FMOD_RESULT eRet = _pSystem->playSound(_pSound, nullptr, AK_FALSE, &_pChannel);
		if (eRet == FMOD_OK)
		{
			if (bLoop)
			{
				_pChannel->setMode(FMOD_LOOP_NORMAL);
			}
			else
			{
				_pChannel->setMode(FMOD_LOOP_OFF);
			}
		}
		return false;
	}
	return true;
}

void Sound::PlayOnce()
{
	FMOD::Channel* pChannel = nullptr;
	FMOD_RESULT eRet = _pSystem->playSound(_pSound, nullptr, AK_FALSE, &pChannel);
}

void Sound::Stop()
{
}

void Sound::Pause()
{
}

void Sound::VolumeUp()
{
}

void Sound::VolumeDown()
{
}

void Sound::CleanUp()
{
	if (_pSound)
	{
		_pSound->release();
		_pSound = nullptr;
	}
}

void Sound::VolumeControl()
{
}
