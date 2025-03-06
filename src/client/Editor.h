#pragma once

class UApplication;
class UEditorCamera;

class UEditor
{
public:
	UEditor();
	virtual ~UEditor();
		
	virtual AkBool Initialize(UApplication* pApp) = 0;
	virtual void BeginEditor() = 0;
	virtual void EndEditor() = 0;
	virtual void Update(const AkF32 fDeltaTime) = 0;
	virtual void Render() = 0;

	UEditorCamera* GetEditorCamera() { return _pEditorCam; }
	UApplication* GetApp() { return _pApp; }

protected:
	void CreateEditorCamera();
	void DestroyEditorCamera();

private:
	void CleanUp();

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;
	UEditorCamera* _pEditorCam = nullptr;
};

