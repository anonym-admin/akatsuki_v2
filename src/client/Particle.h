#pragma once

/*
===========
Particle
===========
*/

class Particle
{
public:
	const AkU32 MAX_COUNT = 1000;

	Particle() = default;
	Particle(const wchar_t* wcFilename);
	virtual ~Particle();

	AkBool Initialize(const wchar_t* wcFilename);
	virtual void Render() = 0;
	virtual void CreateParticles() = 0;
	virtual void Update() = 0;
	virtual void UpdateEditor() = 0;
	virtual void Play(const Vector3* pPos) = 0;
	virtual void Stop();
	virtual void Resume();

	Transform* GetTransform() { return _pTransform; }

private:
	void CleanUp();

protected:
	AkBool _bIsPlay = AK_FALSE;
	Transform* _pTransform = nullptr;
	AkU32 _uParticleCount = 0;
	IParticle* _pParticle = nullptr;
	void* _pDBHandle = nullptr;
	void* _pTexHandle = nullptr;
};
