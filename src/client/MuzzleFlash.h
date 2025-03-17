#pragma once

/*
==================
Muzzle Flash 
==================
*/

class MuzzleFlash
{
public:
	static const AkU32 MAX_PARTICLE_COUNT = 1000;

	MuzzleFlash(const wchar_t* wcTexFilename);
	~MuzzleFlash();

	AkBool Initialize(const wchar_t* wcTexFilename);

	void Update();
	void UpdateEditor();
	void Render();
	void Play(const Vector3* pPos);
	void Stop();

	Transform* GetTransform() { return _pTransform; }

private:
	void CleanUp();

	void CreateTransform();
	void CreateParticle();
	void UpdateParticle();

private:
	Transform* _pTransform = nullptr;
	void* _pTexHandle = nullptr;
	void* _pDBHandle = nullptr;

	IParticle* _pParticle = nullptr;
	
	Particle_t _pParticles[MAX_PARTICLE_COUNT] = {};
};

