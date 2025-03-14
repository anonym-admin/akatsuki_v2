#include "pch.h"
#include "TreeBillboards.h"
#include "BillboardModel.h"

/*
===============
Tree Billboard
===============
*/


TreeBillboard::TreeBillboard()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

TreeBillboard::~TreeBillboard()
{
	CleanUp();
}

AkBool TreeBillboard::Initialize()
{
	
	BillboardVertex_t pVertices[5] = {};
	pVertices[0].vPosition = Vector3(-5.0f, 0.0f, 0.0f);
	pVertices[0].vSize = Vector2(1.0f);
	pVertices[1].vPosition = Vector3(-3.0f, 0.0f, 0.0f);
	pVertices[1].vSize = Vector2(1.0f);
	pVertices[2].vPosition = Vector3(-1.0f, 0.0f, 0.0f);
	pVertices[2].vSize = Vector2(1.0f);
	pVertices[3].vPosition = Vector3(2.0f, 0.0f, 0.0f);
	pVertices[3].vSize = Vector2(1.0f);
	pVertices[4].vPosition = Vector3(4.0f, 0.0f, 0.0f);
	pVertices[4].vSize = Vector2(1.0f);

	BillboardData_t pBillboardData = {};
	pBillboardData.pVertice = pVertices;
	pBillboardData.uPointsNum = _countof(pVertices);
	wcscpy_s(pBillboardData.wcAlbedoTextureFilename, L"../../assets/tree02S.dds");

	_pModel = CreateBillboardModel(&pBillboardData);

	//// Create Model
	//AkU32 uMeshDataNum = 0;
	//MeshData_t* pSquare = GeometryGenerator::MakeSquare(&uMeshDataNum, 1.0f);
	//wcscpy_s(pSquare->wcAlbedoTextureFilename, L"../../assets/tree02S.dds");
	//Vector3 vAlbedo = Vector3(1.0f);
	//Vector3 vEmissvie = Vector3(0.0f);
	//_pModel = CreateBillboardModel(pSquare, 1, &vAlbedo, 0.0f, 1.0f, &vEmissvie);
	//GeometryGenerator::DestroyGeometry(pSquare, 1);

	// Create transform.
	_pTransform = CreateTransform();
	_pTransform->SetPosition(0.0f, 1.5f, 0.0f);

	return AK_TRUE;
}

void TreeBillboard::Update()
{
}

void TreeBillboard::FinalUpdate()
{
	_pTransform->Update();

	_pModel->UpdateWorldRow(&_pTransform->GetWorldTransform());
}

void TreeBillboard::Render()
{
	_pModel->Render();
}

void TreeBillboard::CleanUp()
{
}
