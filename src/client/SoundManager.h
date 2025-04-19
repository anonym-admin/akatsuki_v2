#pragma once

/*
===================
Sound Manager
===================
*/

class Sound;

class SoundManager
{
public:
	SoundManager();
	~SoundManager();

	AkBool Initialize();
	void Update();
	void Render();

	Sound* LoadSound(const char* pFilename);

private:
	void CleanUp();

private:
	FMOD::System* _pSystem = nullptr;
	std::unordered_map<std::wstring, Sound*> _mapSound = {};
};