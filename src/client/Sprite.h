#pragma once

#include "Particle.h"

/*
===================
Sprite Effect
===================
*/

class Sprite : public Particle
{
public:
	Sprite(const wchar_t* wcTexFilename, const Vector2* pMaxFrame);
	~Sprite();

	AkBool Initialize(const Vector2* pMaxFrame);
	virtual void Update() override;
	virtual void UpdateEditor() override;
	virtual void Render() override;
	virtual void CreateParticles() override;
	virtual void Play(const Vector3* pPos) override;

private:
	void CleanUp();

private:
	Vector2 _vMaxFrame = Vector2(0.0f);
	Vector2 _vCurFrame = Vector2(0.0f);

	VertexSize_t* _pVertices = nullptr;
	
	AkF32 _fTime = 0.0f;
	AkF32 _fSpeed = 1.0f;
	AkU32 _uCurFrameCount = 0;
	AkU32 _uMaxFrameCount = 0;

	Vector2 _vSize = Vector2(0.5f);
};

