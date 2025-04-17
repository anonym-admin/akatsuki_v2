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

	AkU32 GetCullObjCount() { return _uCullingCount; }
	AkU32 GetTotalRenderObjCount() { return _uTotalRenderObj; }

private:
	AkBool CheckCube(AkF32 fCenterX, AkF32 fCenterY, AkF32 fCenterZ, AkF32 fRadiusX, AkF32 fRadiusY, AkF32 fRadiusZ);
	AkBool CheckSphere(AkF32 fCenterX, AkF32 fCenterY, AkF32 fCenterZ, AkF32 fRadius);

private:
	Vector4 _pPlane[6] = {};

	AkU32 _uCullingCount = 0;
	AkU32 _uTotalRenderObj = 0;
};

