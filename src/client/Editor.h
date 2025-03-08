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
	virtual void FinalUpdate() = 0;
	virtual void Render() = 0;
	virtual void RenderShadow() = 0;

	void SetName(const wchar_t* wcName) { Name = wcName; }

protected:
	virtual void Load() = 0;
	virtual void Save() = 0;

	Model* CreateModel(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive, AkBool bIsSkinned);
	Camera* CreateCamera();
	Transform* CreateTransform();
	Animation* CreateAnimation(AssetMeshDataContainer_t* pMeshDataContainer, const wchar_t* wcIdleClipName, AkU32 uMaxClipNum);

protected:
	Model* _pTarget = nullptr;
	Camera* _pCamera = nullptr;
	Transform* _pTransform = nullptr;
	Animation* _pAnimation = nullptr;

public:
	const wchar_t* Name;
};

