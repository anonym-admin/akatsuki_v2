#pragma once

/*
===================
Sound Manager
===================
*/

class USound;

class USoundManager
{
public:
	USoundManager();
	~USoundManager();

	AkBool Initialize();
	void Update(const AkF32 fDeltaTime);
	void Render();

	USound* LoadSound(const char* pFilename);

private:
	void CleanUp();

private:
	FMOD::System* _pSystem = nullptr;

	// юс╫ц...
	USound* _pSound = nullptr;

};

/*
===========
Sound
===========
*/

class USound
{
public:
	USound();
	~USound();
	
	AkBool Initialize(FMOD::System* pSys);
	void Update(const AkF32 fDeltaTime);
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

