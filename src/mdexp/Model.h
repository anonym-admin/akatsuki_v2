#pragma once

#include "Type.h"
#include "common/AkMeshData.h"
#include "interface/IRenderer.h"

struct Material
{
	Vector3 vAlbedo = Vector3(1.0f);
	AkF32 fMetallic = 0.0f;
	AkF32 fRoughness = 1.0f;
	Vector3 vEmission = Vector3(0.0f);
};

/*
=========
Model
=========
*/

class Model
{
public:
	Model();
	virtual ~Model();

	bool Init(MeshData_t* pMeshData, int iMeshCount);
	virtual void Update(const float fDT);
	virtual void Render();
	virtual void RenderShadow();

	Material& GetMaterial() { return _pMaterial; }
	Matrix& GetWorldRow() { return _mWorldRow; }

	void SetLeftHandBoneTransform(Matrix mBoneTransform);
	void SetRightHandBoneTransform(Matrix mBoneTransform);

private:
	void CleanUp();
	void UpdateGui();
	void UpdateGizmo();

protected:
	IMeshObject* _pMeshObj = nullptr;
	Material _pMaterial = {};
	Matrix _mWorldRow = Matrix();
	Matrix _mGizmoWorldRow = Matrix();
	Matrix _mFinalWorldRow = Matrix();
	Matrix _mIdentity = Matrix();
	Vector3 _vScale = Vector3(1.0f);
	Vector3 _vPos = Vector3(0.0f);
	Vector3 _vRot = Vector3(0.0f);
	Matrix _mLeftHandBoneTransform = Matrix();
	Matrix _mRightHandBoneTransform = Matrix();
	Matrix _mBoneTransform = Matrix();

public:
	Model* pOwner = nullptr;
	string Name = "";
	string Path = "";
	string FileName = "";
	bool Picked = false;
	bool Pickable = true;
	bool IsWeapon = false;
	DirectX::BoundingBox BndBox = {};
	DirectX::BoundingSphere BndSphere = {};
	bool Shadow = false;
	bool RightHand = false;
	bool LeftHand = false;
	bool IsWire = false;
};