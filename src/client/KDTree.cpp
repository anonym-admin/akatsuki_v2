#include "pch.h"
#include "KDTree.h"
#include "Collider.h"
#include "Actor.h"

/*
=============
KDTree Node
=============
*/

AkI32 CompareX(const void* pTriA, const void* pTriB)
{
	AkTriangle_t* pTri01 = (*((UCollider**)pTriA))->GetTriangle();
	AkTriangle_t* pTri02 = (*((UCollider**)pTriB))->GetTriangle();

	Vector3 vCentroidA = Centroid(pTri01);
	Vector3 vCentroidB = Centroid(pTri02);

	if ((vCentroidA.x - vCentroidB.x) >= 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

AkI32 CompareY(const void* pTriA, const void* pTriB)
{
	AkTriangle_t* pTri01 = (*((UCollider**)pTriA))->GetTriangle();
	AkTriangle_t* pTri02 = (*((UCollider**)pTriB))->GetTriangle();

	Vector3 vCentroidA = Centroid(pTri01);
	Vector3 vCentroidB = Centroid(pTri02);

	if ((vCentroidA.y - vCentroidB.y) >= 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

AkI32 CompareZ(const void* pTriA, const void* pTriB)
{
	AkTriangle_t* pTri01 = (*((UCollider**)pTriA))->GetTriangle();
	AkTriangle_t* pTri02 = (*((UCollider**)pTriB))->GetTriangle();

	Vector3 vCentroidA = Centroid(pTri01);
	Vector3 vCentroidB = Centroid(pTri02);

	if ((vCentroidA.z - vCentroidB.z) >= 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

KDTreeNode_t* BuildKDTree(UActor* pOwner, UCollider** pTriList, AkU32 uTriNum, AkU32 uDepth)
{
	if (0 == uTriNum)
	{
		return nullptr;
	}

	AkU32 uAxis = uDepth % 3;

	if (0 == uAxis)
	{
		qsort(pTriList, uTriNum, sizeof(UCollider*), CompareX);
	}
	else if (1 == uAxis)
	{
		qsort(pTriList, uTriNum, sizeof(UCollider*), CompareY);
	}
	else
	{
		qsort(pTriList, uTriNum, sizeof(UCollider*), CompareZ);
	}

	AkU32 uMedianIndex = uTriNum / 2;

	// Create AABB.
	AkBox_t tBox = {};
	for (AkU32 i = 0; i < uTriNum; i++)
	{
		Expand(&tBox, &pTriList[i]->GetTriangle()->vP[0]);
		Expand(&tBox, &pTriList[i]->GetTriangle()->vP[1]);
		Expand(&tBox, &pTriList[i]->GetTriangle()->vP[2]);
	}

	// Create node.
	KDTreeNode_t* pNode = new KDTreeNode_t;
	pNode->pBox = new UCollider;
	pNode->pBox->Initialize(pOwner, nullptr);
	pNode->pBox->CreateBoundingBox(&tBox.vMin, &tBox.vMax);
	pNode->pTri = pTriList[uMedianIndex];
	pNode->uTriNum++;

	Vector3 vExtent = (tBox.vMax - tBox.vMin);
	Vector3 vCenter = (tBox.vMax + tBox.vMin) * 0.5f;
	pNode->mTransform = Matrix::CreateScale(vExtent) * Matrix::CreateTranslation(vCenter);

	// Recursion.
	AkU32 uLeftTriNum = uMedianIndex;
	AkU32 uRightTriNum = uTriNum - (uMedianIndex + 1);

	pNode->pLeft = BuildKDTree(pOwner, pTriList, uLeftTriNum, uDepth + 1);
	pNode->pRight = BuildKDTree(pOwner, pTriList + uLeftTriNum + 1, uRightTriNum, uDepth + 1);

	return pNode;
}

void DestroyKDTree(KDTreeNode_t* pNode)
{
	if (nullptr == pNode)
	{
		return;
	}

	if (pNode->pBox)
	{
		pNode->pBox->DestroyBoundingBox();

		delete pNode->pBox;
		pNode->pBox = nullptr;
	}

	KDTreeNode_t* pLeft = pNode->pLeft;
	KDTreeNode_t* pRight = pNode->pRight;

	delete pNode;

	DestroyKDTree(pLeft);
	DestroyKDTree(pRight);
}

void RenderKDTreeNode(KDTreeNode_t* pKDTreeNode, IRenderer* pRenderer, IMeshObject* pBoxMesh)
{
	if (nullptr == pKDTreeNode)
	{
		return;
	}

	if (pBoxMesh)
	{
		pRenderer->RenderBasicMeshObject(pBoxMesh, &pKDTreeNode->mTransform);
	}

	RenderKDTreeNode(pKDTreeNode->pLeft, pRenderer, pBoxMesh);
	RenderKDTreeNode(pKDTreeNode->pRight, pRenderer, pBoxMesh);
}

