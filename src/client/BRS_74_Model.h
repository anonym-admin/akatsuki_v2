#pragma once

#include "Model.h"

class UBRS_74_Model : public UModel
{
public:
	UBRS_74_Model();
	~UBRS_74_Model();

	virtual AkBool Initialize(UApplication* pApp);
	virtual void Update(const AkF32 fDeltaTime);
	virtual void RenderShadow() override;
	virtual void Render() override;
	virtual void RenderNormal() override;
	virtual void Release();

	void SetRelativeRotationYaw(AkF32 fRot);
	void SetRelativeRotationPitch(AkF32 fRot);
	void SetRelativeRotationRoll(AkF32 fRot);
	void SetRelativePosition(Vector3 vPos);
	void SetAnimTransform(Matrix* pMat);

	void GetMinMaxPosition(Vector3* pOutMin, Vector3* pOutMax);

private:
	void CleanUp();

	void SetMinMaxVertexPosition(MeshData_t* pMeshData, AkU32 uMeshDataNum);

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;

	Vector3 _vRelativeRot = Vector3(0.0f);
	Vector3 _vRelativePos = Vector3(0.0f);
	Matrix* _pAnimTransform = nullptr;

	Matrix* _pDefaultMat = nullptr;

	Vector3 _vMinVertPos = Vector3(0.0f);
	Vector3 _vMaxVertPos = Vector3(0.0f);
};

