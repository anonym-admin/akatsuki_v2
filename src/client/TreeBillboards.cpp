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
	pVertices[0].vPosition = Vector3(-5.0f, 1.5f, 1025.0f);
	pVertices[0].vSize = Vector2(1.0f);
	pVertices[1].vPosition = Vector3(-5.0f, 1.5f, 1025.0f);
	pVertices[1].vSize = Vector2(1.0f);
	pVertices[2].vPosition = Vector3(-5.0f, 1.5f, 1025.0f);
	pVertices[2].vSize = Vector2(1.0f);
	pVertices[3].vPosition = Vector3(-5.0f, 1.5f, 1025.0f);
	pVertices[3].vSize = Vector2(1.0f);
	pVertices[4].vPosition = Vector3(-5.0f, 1.5f, 1025.0f);
	pVertices[4].vSize = Vector2(1.0f);

	_pModel = CreateBillboardModel(pVertices, 5);

	


	_pTreeTextureArray = GRenderer->CreateTextureFromFile(L"../../assets/treeArray2.dds", AK_TRUE);

	((BillboardModels*)_pModel)->SetTextureArray(_pTreeTextureArray);


	return AK_TRUE;
}

void TreeBillboard::Update()
{
}

void TreeBillboard::FinalUpdate()
{
}

void TreeBillboard::Render()
{
	_pModel->Render();
}

void TreeBillboard::CleanUp()
{
	if (_pTreeTextureArray)
	{
		GRenderer->DestroyTexture(_pTreeTextureArray);
		_pTreeTextureArray = nullptr;
	}
}
