#pragma once

/*
===========
Sound
===========
*/

class Sound
{
public:
	Sound();
	~Sound();
	
	AkBool Initialize(FMOD::System* pSys);
	void Update();
	void Render();

	AkBool Play(AkBool bLoop);
	void PlayOnce();
	void Stop();
	void Pause();
	void VolumeUp();
	void VolumeDown();

	FMOD::Sound** GetSoundAddressOf() { return &_pSound; }

private:
	void CleanUp();
	void VolumeControl();

private:
	FMOD::System* _pSystem = nullptr;
	FMOD::Sound* _pSound = nullptr;
	FMOD::Channel* _pChannel = nullptr;
	AkBool _bPlaying = AK_FALSE;
};

