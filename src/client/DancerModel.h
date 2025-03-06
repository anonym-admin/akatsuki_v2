#pragma once

#include "Model.h"

class UDancerModel : public UModel
{
public:
	UDancerModel();
	~UDancerModel();

	virtual AkBool Initialize(UApplication* pApp);
	virtual void RenderShadow() override;
	virtual void Render() override;
	virtual void RenderNormal() override;
	virtual void Release();

private:
	void CleanUp();

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;
};

