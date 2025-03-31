#pragma once

class PostRenderControl
{
public:
	void RendeGUI();

private:
	AkI32 _iBloomLevel = 4;
	AkF32 _fBloomStrength = 0.05f;

	AkBool _bUseDepthMap = AK_FALSE;
	AkBool _bUseEffect = AK_FALSE;
	AkF32 _fFogStrength = 1.0f;
	AkF32 _fDepthScale = 0.1f;
};

