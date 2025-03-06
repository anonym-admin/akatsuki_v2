#pragma once

#include "Model.h"

class UPlayerModel : public UModel
{
public:
	UPlayerModel();
	~UPlayerModel();

	virtual AkBool Initialize(UApplication* pApp);
	virtual void Render() override;
	virtual void RenderNormal() override;
	virtual void RenderShadow() override;
	virtual void Release();

private:
	void CleanUp();

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;
};