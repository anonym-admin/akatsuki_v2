#pragma once

#include "ModelObject.h"

class Tree : public ModelObject
{
public:
	Tree(const wchar_t* wcScriptFile);
	~Tree();

	AkBool Initialize(const wchar_t* wcScriptFile);
	virtual void RenderGUI() override;
};

