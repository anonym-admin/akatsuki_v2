#pragma once

/*
==========
Light
==========
*/

enum class LIGHT_TYPE
{
	POINT,
	SPOT,
};

class Light
{
public:
	Light(LIGHT_TYPE eType, const Vector3* pRadiance, const Vector3* pPos, AkF32 fRadius, AkBool bShadow);
	~Light();

	AkBool Initiailize(LIGHT_TYPE eType, const Vector3* pRadiance, const Vector3* pPos, AkF32 fRadius, AkBool bShadow);
	void Update();
	void Render();
	void RenderGUI();

	void SetFallOffDistance(AkF32 fStart, AkF32 fEnd);

private:
	void CleanUp();

private:
	IMeshObject* _pMeshObj = nullptr;
	Transform* _pTransform = nullptr;

	LIGHT_TYPE _eType = {};
	AkF32 _fFallOffStart = 0.0f;
	AkF32 _fFallOffEnd = 10.0f;

	Vector3 _vRadiance = Vector3(5.0f);
	AkF32 _fRadius = 0.0f;
};

