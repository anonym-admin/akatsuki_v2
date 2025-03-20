#pragma once

/*
=========
Editor
=========
*/

class Model;
class Camera;
class Transform;

class Editor
{
public:
	Editor() = default;
	virtual ~Editor() = default;

	virtual AkBool BeginEditor() = 0;
	virtual AkBool EndEditor() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void RenderShadow() = 0;
	virtual void RenderGUI() = 0;

	void SetName(const wchar_t* wcName) { Name = wcName; }

protected:
	virtual void Load(const std::wstring& wcFilePath) = 0;
	virtual void Save(const std::wstring& wcFilePath) = 0;

public:
	const wchar_t* Name = nullptr;
};

