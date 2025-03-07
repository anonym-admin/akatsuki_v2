#pragma once

#include "Editor.h"

class EditorMap : public Editor
{
public:
	EditorMap();
	~EditorMap();

	virtual AkBool BeginEditor() override;
	virtual AkBool EndEditor() override;
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

protected:
	virtual void Load() override;
	virtual void Save() override;
};

