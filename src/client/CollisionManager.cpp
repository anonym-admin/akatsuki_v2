#include "pch.h"
#include "CollisionManager.h"
#include "SceneManager.h"
#include "Application.h"
#include "Actor.h"
#include "Collider.h"
#include "Scene.h"
#include "LandScape.h"
#include "RigidBody.h"
#include "Camera.h"
#include "KDTree.h"
#include "WorldMap.h"

#pragma warning(disable : 4302)

/*
=============================
Red Black Tree Nil Node.
=============================
*/

RbTreeNode_t* g_pNil;

/*
=================
CollisionManager
=================
*/

UCollisionManager::UCollisionManager()
{
}

UCollisionManager::~UCollisionManager()
{
	CleanUp();
}

AkBool UCollisionManager::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	g_pNil = RBT_CreateNode(nullptr, nullptr);
	g_pNil->eColor = RbTreeNode_t::BLACK;

	return AK_TRUE;
}

void UCollisionManager::CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE eLeft, GAME_OBJECT_GROUP_TYPE eRight)
{
	AkU32 uRow = (AkU32)eLeft;
	AkU32 uCol = (AkU32)eRight;

	if (uCol < uRow)
	{
		uRow = (AkU32)eRight;
		uCol = (AkU32)eLeft;
	}

	if (_pCollisionCheckBitsTable[uRow] & (1 << uCol))
	{
		_pCollisionCheckBitsTable[uRow] &= (1 << uCol);
	}
	else
	{
		_pCollisionCheckBitsTable[uRow] |= (1 << uCol);
	}
}

void UCollisionManager::Update()
{
	for (AkU32 uRow = 0; uRow < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; uRow++)
	{
		for (AkU32 uCol = 0; uCol < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; uCol++)
		{
			if (_pCollisionCheckBitsTable[uRow] & (1 << uCol))
			{
				if ((AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_MAP == uCol)
				{
					CollisionStaticObjectUpdate((GAME_OBJECT_GROUP_TYPE)uRow);
				}
				else
				{
					CollisionDynamicObjectUpdate((GAME_OBJECT_GROUP_TYPE)uRow, (GAME_OBJECT_GROUP_TYPE)uCol);
				}
			}
		}
	}
}

void UCollisionManager::CleanUp()
{
	if (_pRBTree)
	{
		RBT_DestroyTree(_pRBTree);
		_pRBTree = nullptr;
	}

	if (g_pNil)
	{
		RBT_DestroyNode(g_pNil);
		g_pNil = nullptr;
	}
}

void UCollisionManager::CollisionStaticObjectUpdate(GAME_OBJECT_GROUP_TYPE eLeft)
{
	if (GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_MAP == eLeft || GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_WEAPON == eLeft)
	{
		return;
	}

	USceneManager* pSceneManager = _pApp->GetSceneManager();
	UScene* pCurScence = pSceneManager->GetCurrentScene();
	GameObjContainer_t* pGameObjContainer = pCurScence->GetGroupObject(eLeft);

	List_t* pCur = pGameObjContainer->pGameObjHead;
	while (pCur != nullptr)
	{
		UActor* pObj = (UActor*)pCur->pData;
		UCollider* pObjCollider = pObj->GetCollider();
		if (nullptr == pObjCollider)
		{
			pCur = pCur->pNext;
			continue;
		}

		if (_pMap->UseOptimize())
		{
			TraversKDTree(_pMap->GetKDTreeNode(), pObjCollider);
		}
		else
		{
			Vector3 vCollisionPoint = Vector3(0.0f);
			UCollider** pMapColliderList = _pMap->GetColliderList();
			AkU32 uMapColliderNum = _pMap->GetColliderNum();

			for (AkU32 i = 0; i < uMapColliderNum; i++)
			{
				ColliderID_u ID = {};
				ID.uLeftID = pObjCollider->GetID();
				ID.uRightID = pMapColliderList[i]->GetID();

				RbTreeNode_t* pSearchNode = RBT_SearchNode(_pRBTree, (void*)ID.u64ID);
				if (nullptr == pSearchNode)
				{
					RBT_InsertNode(&_pRBTree, RBT_CreateNode((void*)ID.u64ID, (void*)AK_FALSE));
					pSearchNode = RBT_SearchNode(_pRBTree, (void*)ID.u64ID);
				}

				AkBool bIsCollision = IntersectSphereToTriangle(pObjCollider->GetBoundingSphere(), pMapColliderList[i]->GetTriangle());
				AkBool bPrevCollision = (AkBool)pSearchNode->pData; // #pragma warning(disable : 4302)
				if (bIsCollision)
				{
					if (bPrevCollision)
					{
						pMapColliderList[i]->OnCollision(pObjCollider);
						pObjCollider->OnCollision(pMapColliderList[i]);
					}
					else
					{
						pMapColliderList[i]->OnCollisionEnter(pObjCollider);
						pObjCollider->OnCollisionEnter(pMapColliderList[i]);
						pSearchNode->pData = (void*)AK_TRUE;
					}
				}
				else
				{
					if (bPrevCollision)
					{
						pMapColliderList[i]->OnCollisionExit(pObjCollider);
						pObjCollider->OnCollisionExit(pMapColliderList[i]);
						pSearchNode->pData = (void*)AK_FALSE;
					}
				}
			}
		}

		pCur = pCur->pNext;
	}
}

void UCollisionManager::CollisionDynamicObjectUpdate(GAME_OBJECT_GROUP_TYPE eLeft, GAME_OBJECT_GROUP_TYPE eRight)
{
	USceneManager* pSceneManager = _pApp->GetSceneManager();
	UScene* pCurScence = pSceneManager->GetCurrentScene();

	GameObjContainer_t* pLeftGameObjContainer = pCurScence->GetGroupObject(eLeft);
	GameObjContainer_t* pRightGameObjContainer = pCurScence->GetGroupObject(eRight);

	List_t* pCurLeft = pLeftGameObjContainer->pGameObjHead;
	while (pCurLeft != nullptr)
	{
		UActor* pLeftObj = (UActor*)pCurLeft->pData;
		UCollider* pLeftCollider = pLeftObj->GetCollider();
		if (nullptr == pLeftCollider)
		{
			pCurLeft = pCurLeft->pNext;
			continue;
		}

		List_t* pCurRight = pRightGameObjContainer->pGameObjHead;
		while (pCurRight != nullptr)
		{
			UActor* pRightObj = (UActor*)pCurRight->pData;
			UCollider* pRightCollider = pRightObj->GetCollider();
			if (nullptr == pRightCollider)
			{
				pCurRight = pCurRight->pNext;
				continue;
			}

			if (pLeftCollider == pRightCollider)
			{
				pCurRight = pCurRight->pNext;
				continue;
			}

			ColliderID_u ID = {};
			ID.uLeftID = pLeftCollider->GetID();
			ID.uRightID = pRightCollider->GetID();

			RbTreeNode_t* pSearchNode = RBT_SearchNode(_pRBTree, (void*)ID.u64ID);
			if (nullptr == pSearchNode)
			{
				RBT_InsertNode(&_pRBTree, RBT_CreateNode((void*)ID.u64ID, (void*)AK_FALSE));
				pSearchNode = RBT_SearchNode(_pRBTree, (void*)ID.u64ID);
			}

			AkBool bPrevCollision = (AkBool)pSearchNode->pData; // #pragma warning(disable : 4302)
			if (IsCollision(pLeftCollider, pRightCollider))
			{
				// 현재 충돌 O
				if (bPrevCollision)
				{
					// 이전에 충돌 O
					pLeftCollider->OnCollision(pRightCollider);
					pRightCollider->OnCollision(pLeftCollider);
				}
				else
				{
					// 이전에는 충돌 X
					pLeftCollider->OnCollisionEnter(pRightCollider);
					pRightCollider->OnCollisionEnter(pLeftCollider);
					pSearchNode->pData = (void*)AK_TRUE;
				}
			}
			else
			{
				// 현재 충돌 X
				if (bPrevCollision)
				{
					pLeftCollider->OnCollisionExit(pRightCollider);
					pRightCollider->OnCollisionExit(pLeftCollider);
					pSearchNode->pData = (void*)AK_FALSE;
				}
			}

			pCurRight = pCurRight->pNext;
		}

		pCurLeft = pCurLeft->pNext;
	}
}

void UCollisionManager::CollisionLandScapeUpdate(GAME_OBJECT_GROUP_TYPE eType)
{
}

void UCollisionManager::TraversKDTree(KDTreeNode_t* pNode, UCollider* pCollider)
{
	if (!pNode)
	{
		return;
	}

	ColliderID_u ID = {};
	ID.uLeftID = pCollider->GetID();
	ID.uRightID = pNode->pTri->GetID();

	RbTreeNode_t* pSearchNode = RBT_SearchNode(_pRBTree, (void*)ID.u64ID);
	if (nullptr == pSearchNode)
	{
		RBT_InsertNode(&_pRBTree, RBT_CreateNode((void*)ID.u64ID, (void*)AK_FALSE));
		pSearchNode = RBT_SearchNode(_pRBTree, (void*)ID.u64ID);
	}

	AkBool bIntersectSphereToTri = AK_FALSE;
	AkBool bIntersectSphereToBox = IntersectSphereToBox(pCollider->GetBoundingSphere(), pNode->pBox->GetBoundingBox());
	AkBool bPrevCollision = (AkBool)pSearchNode->pData; // #pragma warning(disable : 4302)
	if (bIntersectSphereToBox)
	{
		TraversKDTree(pNode->pLeft, pCollider);
		TraversKDTree(pNode->pRight, pCollider);

		if (AK_FALSE == *(pNode->pTri->_pDraw))
		{
			return;
		}

		bIntersectSphereToTri = IntersectSphereToTriangle(pCollider->GetBoundingSphere(), pNode->pTri->GetTriangle());
		if (bIntersectSphereToTri)
		{
			if (bPrevCollision)
			{
				pNode->pTri->OnCollision(pCollider);
				pCollider->OnCollision(pNode->pTri);
			}
			else
			{
				pNode->pTri->OnCollisionEnter(pCollider);
				pCollider->OnCollisionEnter(pNode->pTri);
				pSearchNode->pData = (void*)AK_TRUE;
			}
		}
		else
		{
			if (bPrevCollision)
			{
				pNode->pTri->OnCollisionExit(pCollider);
				pCollider->OnCollisionExit(pNode->pTri);
				pSearchNode->pData = (void*)AK_FALSE;
			}
		}
	}
	else
	{
		if (bPrevCollision)
		{
			pNode->pTri->OnCollisionExit(pCollider);
			pCollider->OnCollisionExit(pNode->pTri);
			pSearchNode->pData = (void*)AK_FALSE;
		}
	}
}

AkBool UCollisionManager::IsCollision(UCollider* pLeft, UCollider* pRight)
{
	// 움직이는 물체끼리의 충돌
	COLLIDER_SHAPE_TYPE eLeftType = pLeft->GetShapeType();
	COLLIDER_SHAPE_TYPE eRightType = pRight->GetShapeType();
	AkBool bIntersect = AK_FALSE;

	if (COLLIDER_SHAPE_TYPE::COLLIDER_SHAPE_SPHERE == eLeftType)
	{
		if (COLLIDER_SHAPE_TYPE::COLLIDER_SHAPE_SPHERE == eRightType)
		{
			AkSphere_t* pSphereA = pLeft->GetBoundingSphere();
			AkSphere_t* pSphereB = pRight->GetBoundingSphere();

#ifdef _DEBUG
			if (!pSphereA || !pSphereB)
			{
				__debugbreak();
				return AK_FALSE;
			}
#endif

			bIntersect = IntersectSphereToSphere(pSphereA, pSphereB);
		}

		if (COLLIDER_SHAPE_TYPE::COLLIDER_SHAPE_BOX == eRightType)
		{
			AkSphere_t* pSphere = pLeft->GetBoundingSphere();
			AkBox_t* pBox = pRight->GetBoundingBox();

#ifdef _DEBUG
			if (!pSphere || !pBox)
			{
				__debugbreak();
				return AK_FALSE;
			}
#endif

			bIntersect = IntersectSphereToBox(pSphere, pBox);
		}
	}

	return bIntersect;
}
