#include "pch.h"
#include "TreeBillboards.h"
#include "BillboardModel.h"

/*
===============
Tree Billboard
===============
*/


Billboard::Billboard(const wchar_t* wcTexArray, VertexSize_t* pVertices, AkU32 uNum)
{
	if (!Initialize(wcTexArray, pVertices, uNum))
	{
		__debugbreak();
	}
}

Billboard::~Billboard()
{
	CleanUp();
}

AkBool Billboard::Initialize(const wchar_t* wcTexArray, VertexSize_t* pVertices, AkU32 uNum)
{
	// Random Location.
	BillboardData_t pBillboardData = {};
	pBillboardData.pVertice = pVertices;
	pBillboardData.uPointsNum = uNum;
	wcscpy_s(pBillboardData.wcArrayFilename, wcTexArray);

	_pModel = CreateBillboardModel(&pBillboardData);

	// Create transform.
	_pTransform = CreateTransform();
	_pTransform->SetPosition(0.0f, 0.5f, 0.0f);

	return AK_TRUE;
}

void Billboard::Update()
{
}

void Billboard::FinalUpdate()
{
	_pTransform->Update();

	_pModel->UpdateWorldRow(&_pTransform->GetWorldTransform());
}

void Billboard::RenderShadowMaps()
{
	_pModel->RenderShadowMaps();
}

void Billboard::Render()
{
	_pModel->Render();
}

void Billboard::CleanUp()
{
}
