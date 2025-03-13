#pragma once

class PostRenderControl
{
public:
	void RenderPost();

private:
	AkI32 _iBloomLevel = 4;
	AkF32 _fBloomStrength = 0.05f;
};

