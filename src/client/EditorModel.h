#pragma once

#include "Editor.h"

class ModelExporter;
class ModelImporter;

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

	void CreateModel(const std::wstring& wcName, const std::wstring& wcExt);
	void CreateAnimation(const std::wstring& wcName, const std::wstring& wcClip);

private:
	AkI32 _iExportType = -1;

	ModelExporter* _pExporter = nullptr;
};

