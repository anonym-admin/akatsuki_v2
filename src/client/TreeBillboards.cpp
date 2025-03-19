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
	VertexSize_t pVertices[5] = {};
	pVertices[0].vPosition = Vector4(-22.5f, 0.0f, 35.0f, 1.0f);
	pVertices[0].vSize = Vector2(1.0f);
	pVertices[1].vPosition = Vector4(-21.5f, 0.0f, 35.0f, 1.0f);
	pVertices[1].vSize = Vector2(1.0f);
	pVertices[2].vPosition = Vector4(-20.0f, 0.0f, 35.0f, 1.0f);
	pVertices[2].vSize = Vector2(1.0f);
	pVertices[3].vPosition = Vector4(-19.0f, 0.0f, 35.0f, 1.0f);
	pVertices[3].vSize = Vector2(1.0f);
	pVertices[4].vPosition = Vector4(-18.0f, 0.0f, 35.0f, 1.0f);
	pVertices[4].vSize = Vector2(1.0f);

	BillboardData_t pBillboardData = {};
	pBillboardData.pVertice = pVertices;
	pBillboardData.uPointsNum = _countof(pVertices);
	wcscpy_s(pBillboardData.wcArrayFilename, L"../../assets/treeArray2.dds");

	_pModel = CreateBillboardModel(&pBillboardData);

	// Create transform.
	_pTransform = CreateTransform();
	_pTransform->SetPosition(0.0f, 0.5f, 0.0f);

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
