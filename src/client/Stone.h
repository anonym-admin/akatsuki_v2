#pragma once

#include "ModelObject.h"

class Stone : public ModelObject
{
public:
	Stone(const wchar_t* wcScriptFile);
	Stone(const Stone& Other);
	~Stone();

	AkBool Initialize(const wchar_t* wcScriptFile);
	virtual void RenderGUI() override;
};

