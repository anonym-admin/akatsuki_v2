#pragma once

#include "Particle.h"

/*
===========
Spark
===========
*/

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

	Spark(const wchar_t* wcTexFilename, ParticleInfo_t* pInfo);
	Spark(const wchar_t* wcFxFilename);
	~Spark();

	AkBool Initialize(ParticleInfo_t* pInfo);
	virtual void Render() override;
	virtual void CreateParticles() override;
	virtual void Update() override;
	virtual void UpdateEditor() override;
	virtual void Play(const Vector3* pPos) override;

	void UpdateParticle();
	void SetDirectionByShape(AkI32 iShape, AkI32 i);

	void Load(const wchar_t* wcFxFilename);

private:
	void CleanUp();

	void DestroyParticles();

private:
	AkF32 fTime = 0.0f;
	AkF32 fDuration = 0.0f;
	Vector2 vStartSize = Vector2(0.0f);
	Vector3 vStartDirection = Vector3(0.0f);
	AkF32 fSizeOverLifeTime = 0.0f;
	Vector3 vRotOverLifeTime = Vector3(0.0f);
	Vector4 vTotalColor = Vector4(0.0f);
	Vector4 vColorOverLifeTime = Vector4(0.0f);

	VertexParticle_t* _pVertices = nullptr;
	ParticleInfo_t* _pInfo = nullptr;

	const char* _pTexNames[4] = { "image1", "image2", "image3", "image4" };
	AkI32 _iSelectName = -1;

	AkBool _bLoad = AK_FALSE;
};

