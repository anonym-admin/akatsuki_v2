#pragma once

#include "Editor.h"

class EditorModel : public Editor
{
public:
	EditorModel();
	~EditorModel();

	virtual AkBool BeginEditor() override;
	virtual AkBool EndEditor() override;
	virtual void Update() override;
	virtual void FinalUpdate() override;
	virtual void Render() override;
	virtual void RenderShadow() override;

private:
	virtual void Load(const std::wstring& wcFilePath) override;
	virtual void Save(const std::wstring& wcFilePath) override;

private:

};

