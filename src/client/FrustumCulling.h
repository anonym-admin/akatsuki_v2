#pragma once

class FrustumCulling
{
public:
	FrustumCulling();
	~FrustumCulling();

	AkBool Initialize();
	AkU32 Process();
	void Render();
	void RenderGUI();

private:
	AkBool CheckCube(AkF32 fCenterX, AkF32 fCenterY, AkF32 fCenterZ, AkF32 fRadiusX, AkF32 fRadiusY, AkF32 fRadiusZ);
	AkBool CheckSphere(AkF32 fCenterX, AkF32 fCenterY, AkF32 fCenterZ, AkF32 fRadius);

private:
	Vector4 _pPlane[6] = {};
};

