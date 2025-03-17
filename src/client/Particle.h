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

	Particle(const wchar_t* wcFilename);
	virtual ~Particle();

	AkBool Initialize(const wchar_t* wcFilename);
	virtual void Render() = 0;
	virtual void CreateParticles() = 0;
	virtual void Update() = 0;
	virtual void UpdateEditor() = 0;
	virtual void Play(const Vector3* pPos) = 0;
	virtual void Stop();
	virtual void Pause();
	virtual void Resume();

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

class Spark : public Particle
{
public:
	enum SHAPE
	{
		SHPERE,
		CIRCLE,
		CONE,
	};

	struct ParticleInfo_t
	{
		AkI32 iShape = -1;
		AkBool bLoop = AK_TRUE;
		AkU32 uCount = 100;
		AkF32 fDuration = 3.0f;
		Vector2 vStartLifeTime = Vector2(1.0f, 3.0f);
		Vector2 vStartSpeed = Vector2(1.0f, 3.0f);
		Vector2 vStartRadius = Vector2(1.0f, 5.0f);
		Vector2 vStartSize = Vector2(0.1f, 1.0f);
		Vector4 vStartColor[2] = { { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };
		Vector4 vColorOverLifeTime = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
		AkF32 fSizeOverLifeTime = 0.0f;
		Vector3 vRotOverLifeTime = Vector3(0.0f);
	};

	Spark(const wchar_t* wcFilename, ParticleInfo_t* pInfo);
	~Spark();

	AkBool Initialize(ParticleInfo_t* pInfo);
	virtual void Render() override;
	virtual void CreateParticles() override;
	virtual void Update() override;
	virtual void UpdateEditor() override;
	virtual void Play(const Vector3* pPos) override;

	void UpdateParticle();
	void SetDirectionByShape(AkI32 iShape, AkI32 i);

private:
	void CleanUp();

private:
	AkF32 fTime = 0.0f;
	AkF32 fDuration = 0.0f;
	Vector2 vStartSize = Vector2(0.0f);
	Vector3 vStartDirection = Vector3(0.0f);
	AkF32 fSizeOverLifeTime = 0.0f;
	Vector3 fRotOverLifeTime = Vector3(0.0f);
	Vector4 fColorOverLifeTime = Vector4(0.0f);

	VertexParticle_t* _pVertices = nullptr;
	ParticleInfo_t* _pInfo = nullptr;
};

