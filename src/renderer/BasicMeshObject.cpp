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
ID3D12RootSignature* FBasicMeshObject::sm_pRootSignature;
ID3D12PipelineState* FBasicMeshObject::sm_pBasicSolidPSO;
ID3D12PipelineState* FBasicMeshObject::sm_pBasicWirePSO;
ID3D12PipelineState* FBasicMeshObject::sm_pNormalPSO;
ID3D12PipelineState* FBasicMeshObject::sm_pDepthOnlyPSO;

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
	pGlobalConstantBuffer->fStrengthIBL = _pRenderer->GetIBLStrength();

	//AkU32 uLightNum = 0;
	//Light_t* pLights = _pRenderer->GetLights(&uLightNum);
	//memcpy(pGlobalConstantBuffer->tLights, pLights, sizeof(Light_t) * uLightNum);

	Light_t tLight;
	_pRenderer->GetGlobalLight(&tLight);
	memcpy(pGlobalConstantBuffer->tLights, &tLight, sizeof(Light_t));

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
	pCmdList->SetGraphicsRootSignature(sm_pRootSignature);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(_bIsWire ? sm_pBasicWirePSO : sm_pBasicSolidPSO); // Wire Frame.

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CD3DX12_GPU_DESCRIPTOR_HANDLE hGPUforMeshes(hGPU, DESCRIPTOR_COUNT_PER_OBJ, uDescriptorSize);
	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		// Draw Mesh(root param 1)
		pCmdList->SetGraphicsRootDescriptorTable(1, hGPUforMeshes);
		hGPUforMeshes.Offset(DESCRIPTOR_COUNT_PER_MESH, uDescriptorSize);

		pCmdList->IASetVertexBuffers(0, 1, &_pMeshes[i].tVBView);
		pCmdList->IASetIndexBuffer(&_pMeshes[i].tIBView);
		pCmdList->DrawIndexedInstanced(_pMeshes[i].uIndexCountPerInstance, 1, 0, 0, 0);
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
	pCmdList->SetGraphicsRootSignature(sm_pRootSignature);
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

void FBasicMeshObject::DrawShadow(ID3D12GraphicsCommandList* pCmdList, const Matrix* pWorldMat)
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	FDescriptorPool* pDescriptorPool = _pRenderer->GetDescriptorPool(0);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();
	FConstantBufferPool* pGlobalCBPool = _pRenderer->GetConstantBufferPool(0, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_GLOBAL);
	FConstantBufferPool* pMeshCBPool = _pRenderer->GetConstantBufferPool(0, CONSTANT_BUFFER_TYPE::CONSTANT_BUFFER_TYPE_MESH);
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

	// Per Obj (b1).
	pDevice->CopyDescriptorsSimple(1, hDest, pMeshCBContainer->hCPU, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	hDest.Offset(1, uDescriptorSize);

	// Set RootSignature.
	pCmdList->SetGraphicsRootSignature(sm_pRootSignature);
	pCmdList->SetDescriptorHeaps(1, &pDescriptorHeap);
	pCmdList->SetPipelineState(sm_pDepthOnlyPSO);

	// Obj (root param 0)
	pCmdList->SetGraphicsRootDescriptorTable(0, hGPU);
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (AkU32 i = 0; i < _uMeshNum; i++)
	{
		pCmdList->IASetVertexBuffers(0, 1, &_pMeshes[i].tVBView);
		pCmdList->IASetIndexBuffer(&_pMeshes[i].tIBView);
		pCmdList->DrawIndexedInstanced(_pMeshes[i].uIndexCountPerInstance, 1, 0, 0, 0);
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
			_pMeshes[i].pAldedoTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcAlbedoTextureFilename, AK_TRUE));
			_pMaterials[i].uUseAlbedoMap = AK_TRUE;
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
			_pMaterials[i].uUseEimissiveMap = AK_TRUE;
		}
		//// Height
		//if (!wcscmp(pMeshData[i].wcHeightTextureFilename, L""))
		//{
		//	_pMeshes[i].pEmissiveTextureHandle = pTextureManager->CreateNullTexture();
		//}
		//else
		//{
		//	_pMeshes[i].pEmissiveTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcHeightTextureFilename, AK_FALSE));
		//}
		
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

		//// Opacity
		//if (!wcscmp(pMeshData[i].wcOpacityTextureFilename, L""))
		//{
		//	_pMeshes[i].pEmissiveTextureHandle = pTextureManager->CreateNullTexture();
		//}
		//else
		//{
		//	_pMeshes[i].pEmissiveTextureHandle = reinterpret_cast<TextureHandle_t*>(_pRenderer->CreateTextureFromFile(pMeshData[i].wcRoughnessTextureFilename, AK_FALSE));
		//	_pMaterials[i].uUseEimissiveMap = AK_TRUE;
		//}

		//// For Debuging.
		//printf("MeshObj Albedo [%p]\n", _pMeshes[i].pAldedoTextureHandle);
		//printf("MeshObj Normal [%p]\n", _pMeshes[i].pNormalTextureHandle);
		//printf("MeshObj Emissive [%p]\n", _pMeshes[i].pEmissiveTextureHandle);
		//printf("MeshObj Metallic [%p]\n", _pMeshes[i].pMetallicTextureHandle);
		//printf("MeshObj Roughness [%p]\n", _pMeshes[i].pRoughnessTextureHandle);
		//printf("MeshObj AO [%p]\n", _pMeshes[i].pAoTextureHandle);
	}

	return AK_TRUE;
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

AkBool FBasicMeshObject::CreateRootSignature()
{
	ID3D12Device* pDevice = _pRenderer->GetDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	// Object - CBV - RootParam(0)
	// {
	//   Mesh 0 - SRV[0] - RootParam(1) - Draw()
	//   Mesh 1 - SRV[1] - RootParam(1) - Draw()
	//   Mesh 2 - SRV[2] - RootParam(1) - Draw()
	//   Mesh 3 - SRV[3] - RootParam(1) - Draw()
	//   Mesh 4 - SRV[4] - RootParam(1) - Draw()
	//   Mesh 5 - SRV[5] - RootParam(1) - Draw()
	// }

	CD3DX12_DESCRIPTOR_RANGE tRangesPerObj[1] = {};
	tRangesPerObj[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0);	// b0, b1: Constant Buffer View per Object.

	CD3DX12_DESCRIPTOR_RANGE tRangesPerTriGroup[4] = {};
	tRangesPerTriGroup[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	// b2: Constant Buffer View per Mesh
	tRangesPerTriGroup[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0);	// t0~t5 : Shader Resource View(Tex) per Mesh.
	tRangesPerTriGroup[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 11);	// t10, t11, t12, t13 : Shader Resource View(Tex) per Mesh. (IBL Texture)
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
	//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	tRootSignatureDesc.Init(_countof(tRootParameters), tRootParameters, _countof(pSamplerDesc), pSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	if (FAILED(D3D12SerializeRootSignature(&tRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
	{
		__debugbreak();
		return AK_FALSE;
	}

	if (FAILED(pDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&sm_pRootSignature))))
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

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	AkU32 uCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	AkU32 uCompileFlags = 0;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"../../shader/BasicShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pBasicVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/BasicShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pBasicPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/NormalShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pNormalVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/NormalShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_0", uCompileFlags, 0, &pNormalGS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/NormalShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pNormalPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/DepthOnlyShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", uCompileFlags, 0, &pDepthOnlyVS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"../../shader/DepthOnlyShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", uCompileFlags, 0, &pDepthOnlyPS, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC tInputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 24,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC tPsoDesc = {};
	tPsoDesc.InputLayout = { tInputElementDescs, _countof(tInputElementDescs) };
	tPsoDesc.pRootSignature = sm_pRootSignature;
	tPsoDesc.VS = CD3DX12_SHADER_BYTECODE(pBasicVS->GetBufferPointer(), pBasicVS->GetBufferSize());
	tPsoDesc.PS = CD3DX12_SHADER_BYTECODE(pBasicPS->GetBufferPointer(), pBasicPS->GetBufferSize());
	tPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	tPsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	tPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	tPsoDesc.DepthStencilState.StencilEnable = FALSE;
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	tPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	tPsoDesc.SampleMask = UINT_MAX;
	tPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	tPsoDesc.NumRenderTargets = 1;
	tPsoDesc.RTVFormats[0] = _pRenderer->GetFloatRTVFormat();
	tPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tPsoDesc.SampleDesc.Count = 1;
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
	if (FAILED(pDevice->CreateGraphicsPipelineState(&tPsoDesc, IID_PPV_ARGS(&sm_pDepthOnlyPSO))))
	{
		__debugbreak();
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
	if (sm_pRootSignature)
	{
		sm_pRootSignature->Release();
		sm_pRootSignature = nullptr;
	}
}

void FBasicMeshObject::DestroyPipelineState()
{
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
