#pragma once

class PostProcess
{
public:
	void RenderPost();

private:
	AkI32 _iBloomLevel = 4;
	AkF32 _fBloomStrength = 0.5f;
};

