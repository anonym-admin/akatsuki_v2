#pragma once

#include "Editor.h"
#include "Spark.h"

/*
=====================
Particle Editor
=====================
*/

class Particle;

class EditorParticle : public Editor
{
public:
	EditorParticle();
	virtual ~EditorParticle();

	AkBool Initialize();
	virtual AkBool BeginEditor() override;
	virtual AkBool EndEditor() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void RenderShadowMaps() override;
	virtual void RenderGUI() override;

	virtual void Load(const std::wstring& wcFilePath) override;
	virtual void Save(const std::wstring& wcFilePath) override;

private:
	void CleanUp();

	void CreateParticle();
	void DestroyParticle();

	void Play();
	void Stop();

private:
	AkBool _bFPV = AK_FALSE;
	Camera* _pCamera = nullptr;

	Particle* _pParticle = nullptr;
	Spark::ParticleInfo_t _tInfo = {};
	AkI32 _iShape = 0;

	const char* _cItems[4] = { "image1", "image2", "image3", "image4" };
	AkI32 _iSelectedItem = 0;
};

