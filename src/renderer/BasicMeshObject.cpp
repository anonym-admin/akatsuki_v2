#include "pch.h"
#include "BasicMeshObject.h"
#include "Renderer.h"
#include "D3DUtils.h"
#include "ResourceManager.h"
#include "DescriptorPool.h"
#include "ConstantBufferPool.h"
#include "TextureManager.h"

/*
===================
BasicMeshObject
===================
*/

AkU32 FBasicMeshObject::sm_uInitRefCount;
ID3D12RootSignature* FBasicMeshObject::sm_pBasicRS;
ID3D12RootSignature* FBasicMeshObject::sm_pDepthOnlyRS;
ID3D12PipelineState* FBasicMeshObject::sm_pBasicSolidPSO;
ID3D12PipelineState* FBasicMeshObject::sm_pBasicWirePSO;
ID3D12PipelineState* FBasicMeshObject::sm_pDrawMaskedSolidPSO;
ID3D12PipelineState* FBasicMeshObject::sm_pNormalPSO;
ID3D12PipelineState* FBasicMeshObject::sm_pDepthOnlyPSO;
ID3D12PipelineState* FBasicMeshObject::sm_pInstanceSolidPSO;
ID3D12PipelineState* FBasicMeshObject::sm_pInstanceWirePSO;

FBasicMeshObject::FBasicMeshObject()
{
}

FBasicMeshObject::~FBasicMeshObject()
{
	CleanUp();
}

AkBool FBasicMeshObject::Initialize(FRenderer* pRenderer)
{
	AkBool bResult = AK_TRUE;

	_pRenderer = pRenderer;

	bResult = CreateCommonResources();

	return bResult;
}

void FBasicMeshObject::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	FConstantBufferPool* pMaterialCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MATERIAL);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ + (_uMeshNum * DESCRIPTOR_COUNT_PER_MESH);

	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
	{
		__debugbreak();
		return;
	}

	CBContainer_t* pGlobalCBContainer = pGlobalCBPool->Alloc();
	if (!pGlobalCBContainer)
	{
		__debugbreak();
		return;
	}

	GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
	_pRenderer->GetViewPorjMatrix(&pGlobalConstantBuffer->mView, &pGlobalConstantBuffer->mProj);
	_pRenderer->GetCameraPosition(&pGlobalConstantBuffer->vEyeWorld.x, &pGlobalConstantBuffer->vEyeWorld.y, &pGlobalConstantBuffer->vEyeWorld.z);
	pGlobalConstantBuffer->fTime = _pRenderer->GetToltalTime();
	pGlobalConstantBuffer->fStrengthIBL = _fIBLStrength; //_pRenderer->GetIBLStrength();

	AkU32 uPointLightNum = 0;
	AkU32 uSpotLightNum = 0;
	Light_t* pLights = _pRenderer->GetLights(&uPointLightNum, &uSpotLightNum);
	memcpy(pGlobalConstantBuffer->tLights, pLights, sizeof(Light_t) * MAX_LIGHTS_COUNT);

	//Light_t tLight;
	//_pRenderer->GetGlobalLight(&tLight);
	//memcpy(pGlobalConstantBuffer->tLights, &tLight, sizeof(Light_t));

	Matrix mLightView = Matrix();
	Matrix mLightProj = Matrix();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 0);
	pGlobalConstantBuffer->tLights->mViewProj[0] = (mLightView * mLightProj).Transpose();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 1);
	pGlobalConstantBuffer->tLights->mViewProj[1] = (mLightView * mLightProj).Transpose();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 2);
	pGlobalConstantBuffer->tLights->mViewProj[2] = (mLightView * mLightProj).Transpose();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 3);
	pGlobalConstantBuffer->tLights->mViewProj[3] = (mLightView * mLightProj).Transpose();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 4);
	pGlobalConstantBuffer->tLights->mViewProj[4] = (mLightView * mLightProj).Transpose();

	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
	if (!pMeshCBContainer)
	{
		__debugbreak();
		return;
	}

	MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr);
	Matrix mWorldRow = *pWorldMat;
	pMeshConstantBuffer->mWorld = mWorldRow.Transpose();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow.Invert().Transpose();
	pMeshConstantBuffer->mWorldIT = mWorldRow.Transpose();
	pMeshConstantBuffer->fHeightScale = _fHeightScale;
	pMeshConstantBuffer->fWindTrunk = _fWindTrunk;
	pMeshConstantBuffer->fWindLeaves = _fWindLeaves;

	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Per Mesh
	TextureHandle_t* pIrradianceTexHandle = nullptr;
	TextureHandle_t* pSpecularTexHandle = nullptr;
	TextureHandle_t* pBrdfTexHandle = nullptr;
	_pRenderer->GetIBLTexture(&pIrradianceTexHandle, &pSpecularTexHandle, &pBrdfTexHandle);

	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		CBContainer_t* pMaterialCBContainer = pMaterialCBPool->Alloc();
		if (!pMaterialCBContainer)
		{
			__debugbreak();
			return;
		}

		MaterialConstantBuffer_t* pMaterialConstantBuffer = reinterpret_cast<MaterialConstantBuffer_t*>(pMaterialCBContainer->pSystemMemAddr);
		memcpy(pMaterialConstantBuffer, &_pMaterials[i], sizeof(MaterialConstantBuffer_t));

		// Material CB(b2)
		pDevice->CopyDescriptorsSimple(1, hDest, pMaterialCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// Albedo
		TextureHandle_t* pTexHandle = _pMeshes[i].pAldedoTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Normal
		pTexHandle = _pMeshes[i].pNormalTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Emissive
		pTexHandle = _pMeshes[i].pEmissiveTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Metallic
		pTexHandle = _pMeshes[i].pMetallicTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Roughness
		pTexHandle = _pMeshes[i].pRoughnessTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// AO
		pTexHandle = _pMeshes[i].pAoTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Height
		pTexHandle = _pMeshes[i].pHeightTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Irradiance IBL.
		if (pIrradianceTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pIrradianceTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Specular IBL
		if (pSpecularTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pSpecularTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Brdf Tex
		if (pBrdfTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pBrdfTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Shadow Map
		for (AkU32 uCascadeIndex = 0; uCascadeIndex < FRenderer::CASCADE_SHADOW_MAP_LEVEL; uCascadeIndex++)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
			_pRenderer->GetShadowMapSrv(&hSRV, uCascadeIndex);
			if (hSRV.ptr)
			{
				pDevice->CopyDescriptorsSimple(1, hDest, hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			else
			{
				AkI32 a = 3;
			}
			hDest.Offset(1, uDescriptorSize);
		}
	}

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pBasicRS);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	if (_bUseInstance)
		pCmdList->SetPipelineState(_bIsWire ? sm_pInstanceWirePSO : sm_pInstanceSolidPSO);
	else
		pCmdList->SetPipelineState(_bIsWire ? sm_pBasicWirePSO : sm_pBasicSolidPSO); 

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUforMeshes(hGPU, DESCRIPTOR_COUNT_PER_OBJ, uDescriptorSize);
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		if (_pMeshes[i].pHeightTextureHandle->pTextureResource)
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pMeshes[i].pHeightTextureHandle->pTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));

		// Draw Mesh(root param 1)
		pCmdList->SetGraphicsRootDescriptorTable(1, hGPUforMeshes);
		hGPUforMeshes.Offset(DESCRIPTOR_COUNT_PER_MESH, uDescriptorSize);
		
		pCmdList->IASetIndexBuffer(&_pMeshes[i].tIBView);
		if (_bUseInstance)
		{
			D3D12_VERTEX_BUFFER_VIEW pBuffers[] =
			{
				_pMeshes[i].tVBView,
				_tInstVBView,
			};
			pCmdList->IASetVertexBuffers(0, 2, pBuffers);
			pCmdList->DrawIndexedInstanced(_pMeshes[i].uIndexCountPerInstance, _uInstanceCount, 0, 0, 0);
		}
		else
		{
			pCmdList->IASetVertexBuffers(0, 1, &_pMeshes[i].tVBView);
			pCmdList->DrawIndexedInstanced(_pMeshes[i].uIndexCountPerInstance, 1, 0, 0, 0);
		}

		if (_pMeshes[i].pHeightTextureHandle->pTextureResource)
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pMeshes[i].pHeightTextureHandle->pTextureResource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
}

void FBasicMeshObject::DrawNormal(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ;

	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
	{
		__debugbreak();
		return;
	}

	CBContainer_t* pGlobalCBContainer = pGlobalCBPool->Alloc();
	if (!pGlobalCBContainer)
	{
		__debugbreak();
		return;
	}

	GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
	_pRenderer->GetViewPorjMatrix(&pGlobalConstantBuffer->mView, &pGlobalConstantBuffer->mProj);
	_pRenderer->GetCameraPosition(&pGlobalConstantBuffer->vEyeWorld.x, &pGlobalConstantBuffer->vEyeWorld.y, &pGlobalConstantBuffer->vEyeWorld.z);

	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
	if (!pMeshCBContainer)
	{
		__debugbreak();
		return;
	}

	MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr);
	Matrix mWorldRow = *pWorldMat;
	pMeshConstantBuffer->mWorld = mWorldRow.Transpose();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow.Invert().Transpose();
	pMeshConstantBuffer->mWorldIT = mWorldRow.Transpose();

	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pBasicRS);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pNormalPSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		pCmdList->IASetVertexBuffers(0, 1, &_pMeshes[i].tVBView);
		pCmdList->DrawInstanced(_pMeshes[i].uVertexCountPerInstance, 1, 0, 0);
	}
}

void FBasicMeshObject::DrawShadowMaps(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	FConstantBufferPool* pMaterialCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MATERIAL);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ + (_uMeshNum * 2);

	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
	{
		__debugbreak();
		return;
	}

	CBContainer_t* pGlobalCBContainer = pGlobalCBPool->Alloc();
	if (!pGlobalCBContainer)
	{
		__debugbreak();
		return;
	}

	GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
	Matrix mViewRow = Matrix();
	Matrix mProjRow = Matrix();
	_pRenderer->GetShadowViewProjMatrix(&mViewRow, &mProjRow, _pRenderer->GetCascadeIndex() - 1);
	pGlobalConstantBuffer->mView = mViewRow.Transpose();
	pGlobalConstantBuffer->mProj = mProjRow.Transpose();
	pGlobalConstantBuffer->vEyeWorld = Vector3(0.0f);

	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
	if (!pMeshCBContainer)
	{
		__debugbreak();
		return;
	}

	MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr);
	Matrix mWorldRow = *pWorldMat;
	pMeshConstantBuffer->mWorld = mWorldRow.Transpose();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow.Invert().Transpose();
	pMeshConstantBuffer->mWorldIT = mWorldRow.Transpose();
	pMeshConstantBuffer->fHeightScale = _fHeightScale;
	pMeshConstantBuffer->fWindTrunk = _fWindTrunk;
	pMeshConstantBuffer->fWindLeaves = _fWindLeaves;

	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Per Mesh
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		CBContainer_t* pMaterialCBContainer = pMaterialCBPool->Alloc();
		if (!pMaterialCBContainer)
		{
			__debugbreak();
			return;
		}

		MaterialConstantBuffer_t* pMaterialConstantBuffer = reinterpret_cast<MaterialConstantBuffer_t*>(pMaterialCBContainer->pSystemMemAddr);
		memcpy(pMaterialConstantBuffer, &_pMaterials[i], sizeof(MaterialConstantBuffer_t));

		// Material CB(b2)
		pDevice->CopyDescriptorsSimple(1, hDest, pMaterialCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// Height
		TextureHandle_t* pTexHandle = _pMeshes[i].pHeightTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);
	}

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pDepthOnlyRS);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pDepthOnlyPSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUforMeshes(hGPU, DESCRIPTOR_COUNT_PER_OBJ, uDescriptorSize);
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		if (_pMeshes[i].pHeightTextureHandle->pTextureResource)
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pMeshes[i].pHeightTextureHandle->pTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));

		// Draw Mesh(root param 1)
		pCmdList->SetGraphicsRootDescriptorTable(1, hGPUforMeshes);
		hGPUforMeshes.Offset(2, uDescriptorSize);

		pCmdList->IASetVertexBuffers(0, 1, &_pMeshes[i].tVBView);
		pCmdList->IASetIndexBuffer(&_pMeshes[i].tIBView);
		pCmdList->DrawIndexedInstanced(_pMeshes[i].uIndexCountPerInstance, 1, 0, 0, 0);

		if (_pMeshes[i].pHeightTextureHandle->pTextureResource)
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pMeshes[i].pHeightTextureHandle->pTextureResource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
}

void FBasicMeshObject::DrawReflection(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
	pCmdList->OMSetStencilRef(1);

	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(uThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	FConstantBufferPool* pMaterialCBPool = _pRenderer->GetConstantBufferPool(uThreadIndex, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MATERIAL);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ + (_uMeshNum * DESCRIPTOR_COUNT_PER_MESH);

	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
	{
		__debugbreak();
		return;
	}

	CBContainer_t* pGlobalCBContainer = pGlobalCBPool->Alloc();
	if (!pGlobalCBContainer)
	{
		__debugbreak();
		return;
	}

	GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
	_pRenderer->GetRelectionViewProjMatrix(&pGlobalConstantBuffer->mView, &pGlobalConstantBuffer->mProj);
	_pRenderer->GetCameraPosition(&pGlobalConstantBuffer->vEyeWorld.x, &pGlobalConstantBuffer->vEyeWorld.y, &pGlobalConstantBuffer->vEyeWorld.z);
	pGlobalConstantBuffer->fStrengthIBL = _fIBLStrength;

	AkU32 uPointLightNum = 0;
	AkU32 uSpotLightNum = 0;
	Light_t* pLights = _pRenderer->GetLights(&uPointLightNum, &uSpotLightNum);
	memcpy(pGlobalConstantBuffer->tLights, pLights, sizeof(Light_t) * MAX_LIGHTS_COUNT);

	Matrix mLightView = Matrix();
	Matrix mLightProj = Matrix();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 0);
	pGlobalConstantBuffer->tLights->mViewProj[0] = (mLightView * mLightProj).Transpose();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 1);
	pGlobalConstantBuffer->tLights->mViewProj[1] = (mLightView * mLightProj).Transpose();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 2);
	pGlobalConstantBuffer->tLights->mViewProj[2] = (mLightView * mLightProj).Transpose();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 3);
	pGlobalConstantBuffer->tLights->mViewProj[3] = (mLightView * mLightProj).Transpose();
	_pRenderer->GetShadowViewProjMatrix(&mLightView, &mLightProj, 4);
	pGlobalConstantBuffer->tLights->mViewProj[4] = (mLightView * mLightProj).Transpose();

	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
	if (!pMeshCBContainer)
	{
		__debugbreak();
		return;
	}

	MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr);
	Matrix mWorldRow = *pWorldMat;
	pMeshConstantBuffer->mWorld = mWorldRow.Transpose();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow.Invert().Transpose();
	pMeshConstantBuffer->mWorldIT = mWorldRow.Transpose();
	pMeshConstantBuffer->fHeightScale = _fHeightScale;
	pMeshConstantBuffer->fWindTrunk = _fWindTrunk;
	pMeshConstantBuffer->fWindLeaves = _fWindLeaves;

	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Per Mesh
	TextureHandle_t* pIrradianceTexHandle = nullptr;
	TextureHandle_t* pSpecularTexHandle = nullptr;
	TextureHandle_t* pBrdfTexHandle = nullptr;
	_pRenderer->GetIBLTexture(&pIrradianceTexHandle, &pSpecularTexHandle, &pBrdfTexHandle);

	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		CBContainer_t* pMaterialCBContainer = pMaterialCBPool->Alloc();
		if (!pMaterialCBContainer)
		{
			__debugbreak();
			return;
		}

		MaterialConstantBuffer_t* pMaterialConstantBuffer = reinterpret_cast<MaterialConstantBuffer_t*>(pMaterialCBContainer->pSystemMemAddr);
		memcpy(pMaterialConstantBuffer, &_pMaterials[i], sizeof(MaterialConstantBuffer_t));

		// Material CB(b2)
		pDevice->CopyDescriptorsSimple(1, hDest, pMaterialCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// Albedo
		TextureHandle_t* pTexHandle = _pMeshes[i].pAldedoTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Normal
		pTexHandle = _pMeshes[i].pNormalTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Emissive
		pTexHandle = _pMeshes[i].pEmissiveTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Metallic
		pTexHandle = _pMeshes[i].pMetallicTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Roughness
		pTexHandle = _pMeshes[i].pRoughnessTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// AO
		pTexHandle = _pMeshes[i].pAoTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Height
		pTexHandle = _pMeshes[i].pHeightTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Irradiance IBL.
		if (pIrradianceTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pIrradianceTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Specular IBL
		if (pSpecularTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pSpecularTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Brdf Tex
		if (pBrdfTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pBrdfTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);

		// Shadow Map
		for (AkU32 uCascadeIndex = 0; uCascadeIndex < FRenderer::CASCADE_SHADOW_MAP_LEVEL; uCascadeIndex++)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE hSRV = {};
			_pRenderer->GetShadowMapSrv(&hSRV, uCascadeIndex);
			if (hSRV.ptr)
			{
				pDevice->CopyDescriptorsSimple(1, hDest, hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
			else
			{
				AkI32 a = 3;
			}
			hDest.Offset(1, uDescriptorSize);
		}
	}

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pBasicRS);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pDrawMaskedSolidPSO); // Wire Frame.

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUforMeshes(hGPU, DESCRIPTOR_COUNT_PER_OBJ, uDescriptorSize);
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		if (_pMeshes[i].pHeightTextureHandle->pTextureResource)
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pMeshes[i].pHeightTextureHandle->pTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));

		// Draw Mesh(root param 1)
		pCmdList->SetGraphicsRootDescriptorTable(1, hGPUforMeshes);
		hGPUforMeshes.Offset(DESCRIPTOR_COUNT_PER_MESH, uDescriptorSize);

		pCmdList->IASetVertexBuffers(0, 1, &_pMeshes[i].tVBView);
		pCmdList->IASetIndexBuffer(&_pMeshes[i].tIBView);
		pCmdList->DrawIndexedInstanced(_pMeshes[i].uIndexCountPerInstance, 1, 0, 0, 0);

		if (_pMeshes[i].pHeightTextureHandle->pTextureResource)
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pMeshes[i].pHeightTextureHandle->pTextureResource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
}

void FBasicMeshObject::DrawDepthMap(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(0);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(0, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(0, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
	FConstantBufferPool* pMaterialCBPool = _pRenderer->GetConstantBufferPool(0, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MATERIAL);
	AkU32 uDescriptorSize = pDescriptorPool->GetDescriptorTypeSize();

	CD3DX12_CPU_DESCRIPTOR_HANDLE hCPU = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPU = {};
	AkU32 uRequiredDescriptorNum = DESCRIPTOR_COUNT_PER_OBJ + (_uMeshNum * 2);

	if (!pDescriptorPool->AllocDescriptorTable(&hCPU, &hGPU, uRequiredDescriptorNum))
	{
		__debugbreak();
		return;
	}

	CBContainer_t* pGlobalCBContainer = pGlobalCBPool->Alloc();
	if (!pGlobalCBContainer)
	{
		__debugbreak();
		return;
	}

	GlobalConstantBuffer_t* pGlobalConstantBuffer = reinterpret_cast<GlobalConstantBuffer_t*>(pGlobalCBContainer->pSystemMemAddr);
	_pRenderer->GetViewPorjMatrix(&pGlobalConstantBuffer->mView, &pGlobalConstantBuffer->mProj);
	_pRenderer->GetCameraPosition(&pGlobalConstantBuffer->vEyeWorld.x, &pGlobalConstantBuffer->vEyeWorld.y, &pGlobalConstantBuffer->vEyeWorld.z);

	// Per Obj (b0).
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDest(hCPU, 0, uDescriptorSize);
	pDevice->CopyDescriptorsSimple(1, hDest, pGlobalCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	CBContainer_t* pMeshCBContainer = pMeshCBPool->Alloc();
	if (!pMeshCBContainer)
	{
		__debugbreak();
		return;
	}

	MeshConstantBuffer_t* pMeshConstantBuffer = reinterpret_cast<MeshConstantBuffer_t*>(pMeshCBContainer->pSystemMemAddr);
	Matrix mWorldRow = *pWorldMat;
	pMeshConstantBuffer->mWorld = mWorldRow.Transpose();
	mWorldRow.Translation(Vector3(0.0f));
	mWorldRow.Invert().Transpose();
	pMeshConstantBuffer->mWorldIT = mWorldRow.Transpose();
	pMeshConstantBuffer->fHeightScale = _fHeightScale;
	pMeshConstantBuffer->fWindTrunk = _fWindTrunk;
	pMeshConstantBuffer->fWindLeaves = _fWindLeaves;

	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Per Mesh
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		CBContainer_t* pMaterialCBContainer = pMaterialCBPool->Alloc();
		if (!pMaterialCBContainer)
		{
			__debugbreak();
			return;
		}

		MaterialConstantBuffer_t* pMaterialConstantBuffer = reinterpret_cast<MaterialConstantBuffer_t*>(pMaterialCBContainer->pSystemMemAddr);
		memcpy(pMaterialConstantBuffer, &_pMaterials[i], sizeof(MaterialConstantBuffer_t));

		// Material CB(b2)
		pDevice->CopyDescriptorsSimple(1, hDest, pMaterialCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDest.Offset(1, uDescriptorSize);

		// Height
		TextureHandle_t* pTexHandle = _pMeshes[i].pHeightTextureHandle;
		if (pTexHandle)
		{
			pDevice->CopyDescriptorsSimple(1, hDest, pTexHandle->hSRV, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		else
		{
			__debugbreak();
		}
		hDest.Offset(1, uDescriptorSize);
	}

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pDepthOnlyRS);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pDepthOnlyPSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUforMeshes(hGPU, DESCRIPTOR_COUNT_PER_OBJ, uDescriptorSize);
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		if (_pMeshes[i].pHeightTextureHandle->pTextureResource)
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pMeshes[i].pHeightTextureHandle->pTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE));

		// Draw Mesh(root param 1)
		pCmdList->SetGraphicsRootDescriptorTable(1, hGPUforMeshes);
		hGPUforMeshes.Offset(2, uDescriptorSize);

		pCmdList->IASetVertexBuffers(0, 1, &_pMeshes[i].tVBView);
		pCmdList->IASetIndexBuffer(&_pMeshes[i].tIBView);
		pCmdList->DrawIndexedInstanced(_pMeshes[i].uIndexCountPerInstance, 1, 0, 0, 0);

		if (_pMeshes[i].pHeightTextureHandle->pTextureResource)
			pCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_pMeshes[i].pHeightTextureHandle->pTextureResource, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
}

AkBool FBasicMeshObject::CreateMeshBuffers(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	if (uMeshDataNum <= 0)
	{
		__debugbreak();
		return AK_FALSE;
	}

	FTextureManager* pTextureManager = _pRenderer->GetTextureManager();

	_pMeshes = reinterpret_cast<Mesh_t*>(malloc(sizeof(Mesh_t) * uMeshDataNum));
	_pMaterials = reinterpret_cast<MaterialConstantBuffer_t*>(malloc(sizeof(MaterialConstantBuffer_t) * uMeshDataNum));

	memset(_pMeshes, 0, sizeof(Mesh_t) * uMeshDataNum);
	memset(_pMaterials, 0, sizeof(MaterialConstantBuffer_t) * uMeshDataNum);

	_uMeshNum = uMeshDataNum;

	for (AkU32 i = 0; i < uMeshDataNum; i++)
	{
		CreateVertexAndIndexBuffer(pMeshData, i);

		// Albedo
		if (!wcscmp(pMeshData[i].wcAlbedoTextureFilename, L""))
		{
			_pMeshes[i].pAldedoTextureHandle = pTextureManager->CreateNullTexture();
		}
		else
		{
			if (!wcscmp(pMeshData[i].wcOpacityTextureFilename, L""))
			{
				_pMeshes[i].pAldedoTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcAlbedoTextureFilename, AK_TRUE));
				_pMaterials[i].uUseAlbedoMap = AK_TRUE;
			}
			else
			{
				wchar_t wcFileName[MAX_PATH] = {};

				AkU8* pAlbedoImage = nullptr;
				AkU8* pOpacityImage = nullptr;
				AkU32 uWidth0 = 0;
				AkU32 uHeight0 = 0;
				AkU32 uWidth1 = 0;
				AkU32 uHeight1 = 0;

				DXGI_FORMAT Format = {};

				// Albedo
				ReadImage(pMeshData[i].wcAlbedoTextureFilename, &pAlbedoImage, &uWidth0, &uHeight0, &Format);

				// Opacity
				ReadImage(pMeshData[i].wcOpacityTextureFilename, &pOpacityImage, &uWidth1, &uHeight1);

				if (uWidth0 != uWidth1 || uHeight0 != uHeight1)
				{
					__debugbreak();
				}

				std::wstring wcBasePath = GetFilePath(pMeshData[i].wcAlbedoTextureFilename);
				std::wstring wcAlbedoFileName = GetFileNmaeExcludeExt(GetFileName(pMeshData[i].wcAlbedoTextureFilename));
				wcscpy_s(wcFileName, wcBasePath.c_str());
				wcscat_s(wcFileName, wcAlbedoFileName.c_str());
				wcscat_s(wcFileName, L"_opacity.dds");

				for (AkU32 h = 0; h < uHeight0; h++)
				{
					for (AkU32 w = 0; w < uWidth0; w++)
					{
						AkU32 uIdx = h * uWidth0 + w;

						pAlbedoImage[4 * uIdx + 3] = pOpacityImage[4 * uIdx + 0];
					}
				}

				DWORD dwAttr = GetFileAttributes(wcFileName);
				// Write AlbedoOpacityImage	파일이 없는 경우만 dds 로 저장.
				if (INVALID_FILE_ATTRIBUTES == dwAttr)
					SaveDDS(wcFileName, pAlbedoImage, uWidth0, uHeight0, Format);

				if (pAlbedoImage)
				{
					delete[] pAlbedoImage;
					pAlbedoImage = nullptr;
				}
				if (pOpacityImage)
				{
					delete[] pOpacityImage;
					pOpacityImage = nullptr;
				}

				_pMeshes[i].pAldedoTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(wcFileName, AK_TRUE));
				_pMaterials[i].uUseAlbedoMap = AK_TRUE;
			}
		}
		// Normal
		if (!wcscmp(pMeshData[i].wcNormalTextureFilename, L""))
		{
			_pMeshes[i].pNormalTextureHandle = pTextureManager->CreateNullTexture();
		}
		else
		{
			_pMeshes[i].pNormalTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcNormalTextureFilename, AK_FALSE));
			_pMaterials[i].uUseNormalMap = AK_TRUE;
		}
		// Emissive
		if (!wcscmp(pMeshData[i].wcEmissiveTextureFilename, L""))
		{
			_pMeshes[i].pEmissiveTextureHandle = pTextureManager->CreateNullTexture();
		}
		else
		{
			_pMeshes[i].pEmissiveTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcEmissiveTextureFilename, AK_TRUE));
			_pMaterials[i].uUseEmissiveMap = AK_TRUE;
		}
		// Height
		if (!wcscmp(pMeshData[i].wcHeightTextureFilename, L""))
		{
			_pMeshes[i].pHeightTextureHandle = pTextureManager->CreateNullTexture();
		}
		else
		{
			_pMeshes[i].pHeightTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcHeightTextureFilename, AK_FALSE));
			_pMaterials[i].uUseHeightMap = AK_TRUE;
		}
		// Metallic.
		if (!wcscmp(pMeshData[i].wcMetallicTextureFilename, L""))
		{
			_pMeshes[i].pMetallicTextureHandle = pTextureManager->CreateNullTexture();
		}
		else
		{
			_pMeshes[i].pMetallicTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcMetallicTextureFilename, AK_FALSE));
			_pMaterials[i].uUseMetallicMap = AK_TRUE;
		}

		// Roughness
		if (!wcscmp(pMeshData[i].wcRoughnessTextureFilename, L""))
		{
			_pMeshes[i].pRoughnessTextureHandle = pTextureManager->CreateNullTexture();
		}
		else
		{
			_pMeshes[i].pRoughnessTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcRoughnessTextureFilename, AK_FALSE));
			_pMaterials[i].uUseRoughnessMap = AK_TRUE;
		}

		// Ambient Occulusion
		if (!wcscmp(pMeshData[i].wcAoTextureFilename, L""))
		{
			_pMeshes[i].pAoTextureHandle = pTextureManager->CreateNullTexture();
		}
		else
		{
			_pMeshes[i].pAoTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcAoTextureFilename, AK_FALSE));
			_pMaterials[i].uUseAOMap = AK_TRUE;
		}
	}

	return AK_TRUE;
}

AkBool FBasicMeshObject::CreateInstanceBuffers(VertexInstance_t* pInstData, AkU32 uInstDataNum)
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	D3D12_VERTEX_BUFFER_VIEW tVBView = {};
	ID3D12Resource* pInstBuffer = nullptr;

	if (pResourceManager->CreateInstanceBuffer(sizeof(VertexInstance_t), uInstDataNum, &tVBView, &pInstBuffer, pInstData))
	{
		_pInstBuffer = pInstBuffer;
		_tInstVBView = tVBView;
	}

	_bUseInstance = AK_TRUE;
	_uInstanceCount = uInstDataNum;

	return AK_TRUE;
}

void* FBasicMeshObject::CreateDynamicMeshBuffers(Vertex_t* pVertices, AkU32 uVerticeNum, AkU32* pIndices, AkU32 uIndiceNum)
{
	FTextureManager* pTextureManager = _pRenderer->GetTextureManager();
	DynamicVertexBufferHandle_t* pDVHandle = nullptr;

	_pMeshes = reinterpret_cast<Mesh_t*>(malloc(sizeof(Mesh_t)));
	_uMeshNum = 1;

	memset(_pMeshes, 0, sizeof(Mesh_t));

	_pMeshes->pAldedoTextureHandle = pTextureManager->CreateNullTexture();
	_pMeshes->pNormalTextureHandle = pTextureManager->CreateNullTexture();
	_pMeshes->pEmissiveTextureHandle = pTextureManager->CreateNullTexture();
	_pMeshes->pHeightTextureHandle = pTextureManager->CreateNullTexture();
	_pMeshes->pMetallicTextureHandle = pTextureManager->CreateNullTexture();
	_pMeshes->pRoughnessTextureHandle = pTextureManager->CreateNullTexture();
	_pMeshes->pAoTextureHandle = pTextureManager->CreateNullTexture();

	_pMaterials = reinterpret_cast<MaterialConstantBuffer_t*>(malloc(sizeof(MaterialConstantBuffer_t)));
	memset(_pMaterials, 0, sizeof(MaterialConstantBuffer_t));

	return CreateDynamicVertexAndIndexBuffer(pVertices, uVerticeNum, pIndices, uIndiceNum);
}

void FBasicMeshObject::SetTextures(void* pAlbedo, void* pEmissve, void* pHeight, void* pNormal, void* pMetallic, void* pRoughness, void* pAO)
{
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		if (pAlbedo)
		{
			if (_pMeshes[i].pAldedoTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pAldedoTextureHandle);
				_pMeshes[i].pAldedoTextureHandle = nullptr;
			}

			_pMeshes[i].pAldedoTextureHandle = (TextureHandle_t*)pAlbedo;
			_pMaterials[i].uUseAlbedoMap = AK_TRUE;
		}

		if (pEmissve)
		{
			if (_pMeshes[i].pEmissiveTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pEmissiveTextureHandle);
				_pMeshes[i].pEmissiveTextureHandle = nullptr;
			}

			_pMeshes[i].pEmissiveTextureHandle = (TextureHandle_t*)pEmissve;
			_pMaterials[i].uUseEmissiveMap = AK_TRUE;
		}

		if (pHeight)
		{
			if (_pMeshes[i].pHeightTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pHeightTextureHandle);
				_pMeshes[i].pHeightTextureHandle = nullptr;
			}

			_pMeshes[i].pHeightTextureHandle = (TextureHandle_t*)pHeight;
			_pMaterials[i].uUseHeightMap = AK_TRUE;
		}

		if (pNormal)
		{
			if (_pMeshes[i].pNormalTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pNormalTextureHandle);
				_pMeshes[i].pNormalTextureHandle = nullptr;
			}

			_pMeshes[i].pNormalTextureHandle = (TextureHandle_t*)pNormal;
			_pMaterials[i].uUseNormalMap = AK_TRUE;
		}

		if (pMetallic)
		{
			if (_pMeshes[i].pMetallicTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pMetallicTextureHandle);
				_pMeshes[i].pMetallicTextureHandle = nullptr;
			}

			_pMeshes[i].pMetallicTextureHandle = (TextureHandle_t*)pMetallic;
			_pMaterials[i].uUseMetallicMap = AK_TRUE;
		}

		if (pRoughness)
		{
			if (_pMeshes[i].pRoughnessTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pRoughnessTextureHandle);
				_pMeshes[i].pRoughnessTextureHandle = nullptr;
			}

			_pMeshes[i].pRoughnessTextureHandle = (TextureHandle_t*)pRoughness;
			_pMaterials[i].uUseRoughnessMap = AK_TRUE;
		}

		if (pAO)
		{
			if (_pMeshes[i].pAoTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pAoTextureHandle);
				_pMeshes[i].pAoTextureHandle = nullptr;
			}

			_pMeshes[i].pAoTextureHandle = (TextureHandle_t*)pAO;
			_pMaterials[i].uUseAOMap = AK_TRUE;
		}
	}
}

AkBool FBasicMeshObject::UpdateMaterialBuffers(const Vector3* pAlbedoFactor, AkF32 fMetallicFactor, AkF32 fRoughnessFactor, const Vector3* pEmisiionFactor)
{
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		_pMaterials[i].vAlbedoFactor = *pAlbedoFactor;
		_pMaterials[i].fMetallicFactor = fMetallicFactor;
		_pMaterials[i].fRoughnessFactor = fRoughnessFactor;
		_pMaterials[i].vEmissionFactor = *pEmisiionFactor;
	}

	return AK_TRUE;
}

void FBasicMeshObject::DestoryDynamicVertexBuferHandle(void* pDVHandle)
{
	if (pDVHandle)
	{
		delete pDVHandle;
	}
}

HRESULT __stdcall FBasicMeshObject::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOTIMPL;
}

ULONG __stdcall FBasicMeshObject::AddRef(void)
{
	AkU32 uRefCount = ++_uRefCount;
	return uRefCount;
}

ULONG __stdcall FBasicMeshObject::Release(void)
{
	AkU32 uRefCount = --_uRefCount;
	if (!uRefCount)
	{
		delete this;
	}
	return uRefCount;
}

void FBasicMeshObject::CleanUp()
{
	_pRenderer->EnsureCompleted();

	if (_bUseInstance)
	{
		if (_pInstBuffer)
		{
			_pInstBuffer->Release();
			_pInstBuffer = nullptr;
		}
	}

	if (_pMeshes)
	{
		for (AkU32 i = 0; i < _uMeshNum; i++)
		{
			if (_pMeshes[i].pVB)
			{
				_pMeshes[i].pVB->Release();
				_pMeshes[i].pVB = nullptr;
			}
			if (_pMeshes[i].pIB)
			{
				_pMeshes[i].pIB->Release();
				_pMeshes[i].pIB = nullptr;
			}
			if (_pMeshes[i].pAldedoTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pAldedoTextureHandle);
				_pMeshes[i].pAldedoTextureHandle = nullptr;
			}
			if (_pMeshes[i].pNormalTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pNormalTextureHandle);
				_pMeshes[i].pNormalTextureHandle = nullptr;
			}
			if (_pMeshes[i].pEmissiveTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pEmissiveTextureHandle);
				_pMeshes[i].pEmissiveTextureHandle = nullptr;
			}
			if (_pMeshes[i].pMetallicTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pMetallicTextureHandle);
				_pMeshes[i].pMetallicTextureHandle = nullptr;
			}
			if (_pMeshes[i].pRoughnessTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pRoughnessTextureHandle);
				_pMeshes[i].pRoughnessTextureHandle = nullptr;
			}
			if (_pMeshes[i].pAoTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pAoTextureHandle);
				_pMeshes[i].pAoTextureHandle = nullptr;
			}
			if (_pMeshes[i].pHeightTextureHandle)
			{
				_pRenderer->DestroyTexture(_pMeshes[i].pHeightTextureHandle);
				_pMeshes[i].pHeightTextureHandle = nullptr;
			}
		}

		free(_pMeshes);
		_pMeshes = nullptr;
	}
	if (_pMaterials)
	{
		free(_pMaterials);
		_pMaterials = nullptr;
	}

	DestroyCommonResources();
}

void FBasicMeshObject::CreateVertexAndIndexBuffer(MeshData_t* pMeshData, AkU32 uMeshDataIndex)
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	D3D12_VERTEX_BUFFER_VIEW tVBView = {};
	D3D12_INDEX_BUFFER_VIEW tIBView = {};
	ID3D12Resource* pVertexBuffer = nullptr;
	ID3D12Resource* pIndexBuffer = nullptr;

	if (pResourceManager->CreateVertexBuffer(sizeof(Vertex_t), pMeshData[uMeshDataIndex].uVerticeNum, &tVBView, &pVertexBuffer, pMeshData[uMeshDataIndex].pVertices))
	{
		_pMeshes[uMeshDataIndex].pVB = pVertexBuffer;
		_pMeshes[uMeshDataIndex].tVBView = tVBView;
	}

	if (pResourceManager->CreateIndexBuffer(pMeshData[uMeshDataIndex].uIndicesNum, &tIBView, &pIndexBuffer, pMeshData[uMeshDataIndex].pIndices))
	{
		_pMeshes[uMeshDataIndex].pIB = pIndexBuffer;
		_pMeshes[uMeshDataIndex].tIBView = tIBView;
		_pMeshes[uMeshDataIndex].uVertexCountPerInstance = pMeshData[uMeshDataIndex].uVerticeNum;
		_pMeshes[uMeshDataIndex].uIndexCountPerInstance = pMeshData[uMeshDataIndex].uIndicesNum;
	}
}

DynamicVertexBufferHandle_t* FBasicMeshObject::CreateDynamicVertexAndIndexBuffer(Vertex_t* pVertices, AkU32 uVerticeNum, AkU32* pIndices, AkU32 uIndiceNum)
{
	FResourceManager* pResourceManager = _pRenderer->GetResourceManager();
	D3D12_VERTEX_BUFFER_VIEW tVBView = {};
	D3D12_INDEX_BUFFER_VIEW tIBView = {};
	ID3D12Resource* pVertexBuffer = nullptr;
	ID3D12Resource* pIndexBuffer = nullptr;
	DynamicVertexBufferHandle_t* pDVHandle = nullptr;

	if (pResourceManager->CreateDynamicVertexBuffer(sizeof(Vertex_t), uVerticeNum, &tVBView, &pVertexBuffer))
	{
		_pMeshes[0].pVB = pVertexBuffer;
		_pMeshes[0].tVBView = tVBView;

		pDVHandle = new DynamicVertexBufferHandle_t;
		pDVHandle->pUploadBuffer = pVertexBuffer;
		pDVHandle->uSizePerVertex = sizeof(Vertex_t);
		pDVHandle->uVertexNum = uVerticeNum;
	}

	if (pResourceManager->CreateIndexBuffer(uIndiceNum, &tIBView, &pIndexBuffer, pIndices))
	{
		_pMeshes[0].pIB = pIndexBuffer;
		_pMeshes[0].tIBView = tIBView;
		_pMeshes[0].uVertexCountPerInstance = uVerticeNum;
		_pMeshes[0].uIndexCountPerInstance = uIndiceNum;
	}

	return pDVHandle;
}

AkBool FBasicMeshObject::CreateCommonResources()
{
	if (sm_uInitRefCount)
	{
		sm_uInitRefCount++;
		return AK_TRUE;
	}

	if (!CreateRootSignature())
	{
		__debugbreak();
		return AK_FALSE;
	}
	if (!CreatePipelineState())
	{
		__debugbreak();
		return AK_FALSE;
	}

	sm_uInitRefCount++;

	return AK_TRUE;
}

// Object - CBV - RootParam(0)
// {
//   Mesh 0 - SRV[0] - RootParam(1) - Draw()
//   Mesh 1 - SRV[1] - RootParam(1) - Draw()
//   Mesh 2 - SRV[2] - RootParam(1) - Draw()
//   Mesh 3 - SRV[3] - RootParam(1) - Draw()
//   Mesh 4 - SRV[4] - RootParam(1) - Draw()
//   Mesh 5 - SRV[5] - RootParam(1) - Draw()
// }
AkBool FBasicMeshObject::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	// Create Basic Root Signature.
	{
		CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[1] = {};
		tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);	// b0, b1: Constant Buffer View per Object.

		CD3DX12_DESCRIPTOR_RANGE tRangesPerTriGroup[4] = {};
		tRangesPerTriGroup[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	// b2: Constant Buffer View per Mesh
		tRangesPerTriGroup[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 0);	// t0~t6 : Shader Resource View(Tex) per Mesh.
		tRangesPerTriGroup[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 11);	// t11, t12, t13 : Shader Resource View(Tex) per Mesh. (IBL Texture)
		tRangesPerTriGroup[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 15);	// t15, t16, t17, t18, t19 : Shadow Map

		CD3DX12_ROOT_PARAMETER tRootParameters[2] = {};
		tRootParameters[0].InitAsDescriptorTable(_countof(tRangesPerObj), tRangesPerObj, D3D12_SHADER_VISIBILITY_ALL);
		tRootParameters[1].InitAsDescriptorTable(_countof(tRangesPerTriGroup), tRangesPerTriGroup, D3D12_SHADER_VISIBILITY_ALL);

		// sampler
		CD3DX12_STATIC_SAMPLER_DESC pSamplerDesc[7] = {};
		FD3DUtils::GetStaticSamplers(pSamplerDesc);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS tRootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		// Create an empty root signature.
		CD3DX12_ROOT_SIGNATURE_DESC tRootSignatureDesc;
		tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, _countof(pSamplerDesc), pSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pBasicRS))))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (pSignature)
		{
			pSignature->Release();
			pSignature = nullptr;
		}
		if (pError)
		{
			pError->Release();
			pError = nullptr;
		}
	}

	// Create Depth Only Root Signature.
	{
		CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[1] = {};
		tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);	// b0, b1: Constant Buffer View per Object.

		CD3DX12_DESCRIPTOR_RANGE tRangesPerTriGroup[2] = {};
		tRangesPerTriGroup[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	// b2: Constant Buffer View per Mesh
		tRangesPerTriGroup[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// t0: Height Map

		CD3DX12_ROOT_PARAMETER tRootParameters[2] = {};
		tRootParameters[0].InitAsDescriptorTable(_countof(tRangesPerObj), tRangesPerObj, D3D12_SHADER_VISIBILITY_ALL);
		tRootParameters[1].InitAsDescriptorTable(_countof(tRangesPerTriGroup), tRangesPerTriGroup, D3D12_SHADER_VISIBILITY_ALL);

		// sampler
		CD3DX12_STATIC_SAMPLER_DESC pSamplerDesc[7] = {};
		FD3DUtils::GetStaticSamplers(pSamplerDesc);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS tRootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		// Create an empty root signature.
		CD3DX12_ROOT_SIGNATURE_DESC tRootSignatureDesc;
		tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, _countof(pSamplerDesc), pSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pDepthOnlyRS))))
		{
			__debugbreak();
			return AK_FALSE;
		}

		if (pSignature)
		{
			pSignature->Release();
			pSignature = nullptr;
		}
		if (pError)
		{
			pError->Release();
			pError = nullptr;
		}
	}

	return AK_TRUE;
}

AkBool FBasicMeshObject::CreatePipelineState()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();

	ID3DBlob* pBasicVS = nullptr;
	ID3DBlob* pBasicPS = nullptr;
	ID3DBlob* pNormalVS = nullptr;
	ID3DBlob* pNormalGS = nullptr;
	ID3DBlob* pNormalPS = nullptr;
	ID3DBlob* pDepthOnlyVS = nullptr;
	ID3DBlob* pDepthOnlyPS = nullptr;
	ID3DBlob* pGrassVS = nullptr;
	ID3DBlob* pGrassPS = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 uCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	AkU32 uCompileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/Basic.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pBasicVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/Basic.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pBasicPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/Normal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pNormalVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/Normal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_0", uCompileFlags, 0, &pNormalGS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/Normal.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pNormalPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/DepthOnly.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pDepthOnlyVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/DepthOnly.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pDepthOnlyPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/Grass.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pGrassVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/Grass.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pGrassPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC tBasicInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC tGrassInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		// 행렬 하나는 4x4라서 Element 4개 사용 (쉐이더에서는 행렬 하나)
		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }, // 마지막 1은 instance step
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "COLOR", 0, DXGI_FORMAT_R32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tPsoDesc = {};
	tPsoDesc.InputLayout = { tBasicInputElementDescs, _countof(tBasicInputElementDescs) };
	tPsoDesc.pRootSignature = sm_pBasicRS;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pBasicVS->GetBufferPointer(), pBasicVS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pBasicPS->GetBufferPointer(), pBasicPS->GetBufferSize());
	tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tPsoDesc.DepthStencilState.StencilEnable = FALSE;
	tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tPsoDesc.SampleMask = UINT_MAX;
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tPsoDesc.NumRenderTargets = 1;
	tPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tPsoDesc.SampleDesc.Count = _pRenderer->UseMSAA() ? 4 : 1;
	tPsoDesc.SampleDesc.Quality = _pRenderer->UseMSAA() ? _pRenderer->GetNumQualityLevel() - 1 : 0;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pBasicSolidPSO))))
	{
		__debugbreak();
	}

	tPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pBasicWirePSO))))
	{
		__debugbreak();
	}

	tPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pNormalVS->GetBufferPointer(), pNormalVS->GetBufferSize());
	tPsoDesc.GS = CD3DX12_SHADER_BYTECODE(pNormalGS->GetBufferPointer(), pNormalGS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pNormalPS->GetBufferPointer(), pNormalPS->GetBufferSize());
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pNormalPSO))))
	{
		__debugbreak();
	}

	tPsoDesc.pRootSignature = sm_pDepthOnlyRS;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pDepthOnlyVS->GetBufferPointer(), pDepthOnlyVS->GetBufferSize());
	tPsoDesc.GS = {};
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pDepthOnlyPS->GetBufferPointer(), pDepthOnlyPS->GetBufferSize());
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tPsoDesc.NumRenderTargets = 0;
	tPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	tPsoDesc.RasterizerState.DepthBias = 100000;
	tPsoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	tPsoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	tPsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	tPsoDesc.SampleDesc.Count = 1;
	tPsoDesc.SampleDesc.Quality = 0;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pDepthOnlyPSO))))
	{
		__debugbreak();
	}

	// Stencil 에 1로 표기 된 곳을 랜더링.
	tPsoDesc.pRootSignature = sm_pBasicRS;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pBasicVS->GetBufferPointer(), pBasicVS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pBasicPS->GetBufferPointer(), pBasicPS->GetBufferSize());
	tPsoDesc.NumRenderTargets = 1;
	tPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tPsoDesc.SampleDesc.Count = _pRenderer->UseMSAA() ? 4 : 1;
	tPsoDesc.SampleDesc.Quality = _pRenderer->UseMSAA() ? _pRenderer->GetNumQualityLevel() - 1 : 0;
	tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tPsoDesc.RasterizerState.FrontCounterClockwise = TRUE;
	tPsoDesc.DepthStencilState.StencilEnable = TRUE;
	tPsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	const D3D12_DEPTH_STENCILOP_DESC tDefaultStencilOp = 
	{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_EQUAL };
	tPsoDesc.DepthStencilState.FrontFace = tDefaultStencilOp;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pDrawMaskedSolidPSO))))
	{
		__debugbreak();
	}

	// Create the grass pso.
	tPsoDesc.InputLayout = { tGrassInputElementDescs, _countof(tGrassInputElementDescs) };
	tPsoDesc.pRootSignature = sm_pBasicRS;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pGrassVS->GetBufferPointer(), pGrassVS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pGrassPS->GetBufferPointer(), pGrassPS->GetBufferSize());
	tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tPsoDesc.DepthStencilState.StencilEnable = FALSE;
	tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	tPsoDesc.SampleMask = UINT_MAX;
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tPsoDesc.NumRenderTargets = 1;
	tPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tPsoDesc.SampleDesc.Count = _pRenderer->UseMSAA() ? 4 : 1;
	tPsoDesc.SampleDesc.Quality = _pRenderer->UseMSAA() ? _pRenderer->GetNumQualityLevel() - 1 : 0;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pInstanceSolidPSO))))
	{
		__debugbreak();
	}

	tPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pInstanceWirePSO))))
	{
		__debugbreak();
	}

	if (pGrassPS)
	{
		pGrassPS->Release();
		pGrassPS = nullptr;
	}
	if (pGrassVS)
	{
		pGrassVS->Release();
		pGrassVS = nullptr;
	}
	if (pDepthOnlyPS)
	{
		pDepthOnlyPS->Release();
		pDepthOnlyPS = nullptr;
	}
	if (pDepthOnlyVS)
	{
		pDepthOnlyVS->Release();
		pDepthOnlyVS = nullptr;
	}
	if (pNormalPS)
	{
		pNormalPS->Release();
		pNormalPS = nullptr;
	}
	if (pNormalGS)
	{
		pNormalGS->Release();
		pNormalGS = nullptr;
	}
	if (pNormalVS)
	{
		pNormalVS->Release();
		pNormalVS = nullptr;
	}
	if (pBasicVS)
	{
		pBasicVS->Release();
		pBasicVS = nullptr;
	}
	if (pBasicPS)
	{
		pBasicPS->Release();
		pBasicPS = nullptr;
	}

	return AK_TRUE;
}

void FBasicMeshObject::DestroyCommonResources()
{
	if (!sm_uInitRefCount)
	{
		return;
	}

	AkU32 uRefCount = --sm_uInitRefCount;
	if (!uRefCount)
	{
		DestroyPipelineState();
		DestroyRootSignature();
	}
}

void FBasicMeshObject::DestroyRootSignature()
{
	if (sm_pDepthOnlyRS)
	{
		sm_pDepthOnlyRS->Release();
		sm_pDepthOnlyRS = nullptr;
	}
	if (sm_pBasicRS)
	{
		sm_pBasicRS->Release();
		sm_pBasicRS = nullptr;
	}
}

void FBasicMeshObject::DestroyPipelineState()
{
	if (sm_pInstanceSolidPSO)
	{
		sm_pInstanceSolidPSO->Release();
		sm_pInstanceSolidPSO = nullptr;
	}
	if (sm_pInstanceWirePSO)
	{
		sm_pInstanceWirePSO->Release();
		sm_pInstanceWirePSO = nullptr;
	}
	if (sm_pDrawMaskedSolidPSO)
	{
		sm_pDrawMaskedSolidPSO->Release();
		sm_pDrawMaskedSolidPSO = nullptr;
	}
	if (sm_pDepthOnlyPSO)
	{
		sm_pDepthOnlyPSO->Release();
		sm_pDepthOnlyPSO = nullptr;
	}
	if (sm_pNormalPSO)
	{
		sm_pNormalPSO->Release();
		sm_pNormalPSO = nullptr;
	}
	if (sm_pBasicWirePSO)
	{
		sm_pBasicWirePSO->Release();
		sm_pBasicWirePSO = nullptr;
	}
	if (sm_pBasicSolidPSO)
	{
		sm_pBasicSolidPSO->Release();
		sm_pBasicSolidPSO = nullptr;
	}
}

void FBasicMeshObject::SetHeightScale(AkF32 fHeightScale)
{
	_fHeightScale = fHeightScale;
}

void FBasicMeshObject::SetIBLStrength(AkF32 fIBLStrength)
{
	_fIBLStrength = fIBLStrength;
}

/*
================================================
트리를 따로 클래스로 분리할 수도 있지만,
현재는 편의상 Basic Mesh 클래스에서 공통적으로 생성
=================================================
*/

void FBasicMeshObject::SetWindTrunk(AkF32 fWindTrunk)
{
	_fWindTrunk = fWindTrunk;
}

void FBasicMeshObject::SetWindLeaves(AkF32 fWindLeaves)
{
	_fWindLeaves = fWindLeaves;
}

