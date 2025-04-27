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

CollisionManager::CollisionManager()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

CollisionManager::~CollisionManager()
{
	CleanUp();
}

AkBool CollisionManager::Initialize()
{
	g_pNil = RBT_CreateNode(nullptr, nullptr);
	g_pNil->eColor = RbTreeNode_t::BLACK;

	return AK_TRUE;
}

void CollisionManager::CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE eLeft, GAME_OBJECT_GROUP_TYPE eRight)
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

void CollisionManager::Update()
{
	for (AkU32 uRow = 0; uRow < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; uRow++)
	{
		for (AkU32 uCol = 0; uCol < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; uCol++)
		{
			if (_pCollisionCheckBitsTable[uRow] & (1 << uCol))
			{
				CollisionDynamicObjectUpdate((GAME_OBJECT_GROUP_TYPE)uRow, (GAME_OBJECT_GROUP_TYPE)uCol);
			}
		}
	}
}

void CollisionManager::Reset()
{
	memset(_pCollisionCheckBitsTable, 0, sizeof(AkU32) * (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT);
}

void CollisionManager::CleanUp()
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

void CollisionManager::CollisionDynamicObjectUpdate(GAME_OBJECT_GROUP_TYPE eLeft, GAME_OBJECT_GROUP_TYPE eRight)
{
	Scene* pCurScene = GSceneManager->GetCurrentScene();

	if (!pCurScene)
		return;

	GameObjContainer_t* pLeftGameObjContainer = pCurScene->GetGroupObject(eLeft);
	GameObjContainer_t* pRightGameObjContainer = pCurScene->GetGroupObject(eRight);

	List_t* pCurLeft = pLeftGameObjContainer->pGameObjHead;
	while (pCurLeft != nullptr)
	{
		Actor* pLeftObj = (Actor*)pCurLeft->pData;
		Collider* pLeftCollider = pLeftObj->GetCollider();
		if (nullptr == pLeftCollider)
		{
			pCurLeft = pCurLeft->pNext;
			continue;
		}

		// 컬링이 된 오브젝트라면 충돌처리 체크 X
		if (pLeftObj->Cull)
		{
			pCurLeft = pCurLeft->pNext;
			continue;
		}

		List_t* pCurRight = pRightGameObjContainer->pGameObjHead;
		while (pCurRight != nullptr)
		{
			Actor* pRightObj = (Actor*)pCurRight->pData;
			Collider* pRightCollider = pRightObj->GetCollider();
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

			// 컬링이 된 오브젝트라면 충돌처리 체크 X
			if (pRightObj->Cull)
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

AkBool CollisionManager::IsCollision(Collider* pLeft, Collider* pRight)
{
	// Terrain 일 경우 충돌을 했다고 가정.
	if (!wcscmp(pLeft->GetOwner()->Name, L"Terrain") || !wcscmp(pRight->GetOwner()->Name, L"Terrain"))
	{
		return AK_TRUE;
	}

	return pLeft->Intersect(pRight);
}
