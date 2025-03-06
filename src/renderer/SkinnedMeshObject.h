#pragma once

#include "BasicMeshObject.h"

/*
======================
SkinnedMeshObject
======================
*/

class FSkinnedMeshObject : public FBasicMeshObject
{
public:
	static const AkU32 DESCRIPTOR_COUNT_PER_OBJ = FBasicMeshObject::DESCRIPTOR_COUNT_PER_OBJ + 1;
	static const AkU32 MAX_DESCRIPTOR_COUNT_FOR_DRAW = DESCRIPTOR_COUNT_PER_OBJ + (DESCRIPTOR_COUNT_PER_MESH * MAX_MESH_COUNT_PER_OBJ);

	FSkinnedMeshObject();
	~FSkinnedMeshObject();

	void Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat, const Matrix* pBonesTransform);
	void DrawNormal(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat, const Matrix* pBonesTransform);
	void DrawShadow(ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat, const Matrix* pBonesTransform);

	// virtual AkBool CreateMeshBuffers(MeshData_t* pMeshData, AkU32 uMeshDataNum) override;
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override;
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	virtual ULONG __stdcall Release(void) override;

protected:
	virtual void CreateVertexAndIndexBuffer(MeshData_t* pMeshData, AkU32 uMeshDataIndex) override;

private:
	virtual AkBool CreateCommonResources() override;
	virtual AkBool CreateRootSignature() override;
	virtual AkBool CreatePipelineState() override;
	virtual void DestroyCommonResources() override;
	virtual void DestroyRootSignature() override;
	virtual void DestroyPipelineState() override;

private:
	static AkU32 sm_uSkinnedInitRefCount;
	static ID3D12RootSignature* sm_pRootSignature;
	static ID3D12PipelineState* sm_pSkinnedSolidPSO;
	static ID3D12PipelineState* sm_pSkinnedWirePSO;
	static ID3D12PipelineState* sm_pSkinnedNormalPSO;
	static ID3D12PipelineState* sm_pSkinnedDepthOnlyPSO;
};

