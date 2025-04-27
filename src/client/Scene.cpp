#include "pch.h"
#include "Scene.h"
#include "Actor.h"
#include "Application.h"
#include "FrustumCulling.h"

/*
==================
Scene base class
==================
*/

Scene::~Scene()
{
	CleanUp();
}

void Scene::Update()
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();

	// Update game obj.
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				Actor* pActor = reinterpret_cast<Actor*>(pCur->pData);
				pActor->Update();

				pCur = pCur->pNext;
			}
		}
	}
}

void Scene::FinalUpdate()
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();

	// Final Update game obj.
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				Actor* pActor = reinterpret_cast<Actor*>(pCur->pData);
				pActor->FinalUpdate();

				pCur = pCur->pNext;
			}
		}
	}
}

void Scene::RenderDepthMap()
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();

	// Render Game Obj Depth Map.
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				Actor* pActor = reinterpret_cast<Actor*>(pCur->pData);

				if (!pActor->Cull)
					pActor->RenderDepthMap();

				pCur = pCur->pNext;
			}
		}
	}
}

void Scene::RenderShadowMaps()
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();

	// Render Game Obj Shadow Maps
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				Actor* pActor = reinterpret_cast<Actor*>(pCur->pData);

				if (!pActor->Cull)
					pActor->RenderShadowMaps();

				pCur = pCur->pNext;
			}
		}
	}
}

void Scene::Render()
{
	GameObjContainer_t** pGameObjContainerList = GetAllGameObject();

	// Render game obj.
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; i++)
	{
		if (pGameObjContainerList[i])
		{
			List_t* pCur = pGameObjContainerList[i]->pGameObjHead;
			while (pCur != nullptr)
			{
				Actor* pActor = reinterpret_cast<Actor*>(pCur->pData);
				
				if(!pActor->Cull)
					pActor->Render();

				pCur = pCur->pNext;
			}
		}
	}
}

void Scene::AddGameObject(GAME_OBJECT_GROUP_TYPE eGameObjType, Actor* pGameObj)
{
	GameObjContainer_t* pGameObjContainer = _pGameObjContainerList[(AkU32)eGameObjType];

	if (pGameObjContainer)
	{
		LL_PushBack(&pGameObjContainer->pGameObjHead, &pGameObjContainer->pGameObjTail, &pGameObj->tLink);
	}
	else
	{
		pGameObjContainer = AllocGameObjectContainer();
		LL_PushBack(&pGameObjContainer->pGameObjHead, &pGameObjContainer->pGameObjTail, &pGameObj->tLink);

		_pGameObjContainerList[(AkU32)eGameObjType] = pGameObjContainer;
	}

	_uGameObjNum++;
}

AkU32 Scene::GetRenderObjNum()
{
	return _uGameObjNum - _uCullObjNum;
}

void Scene::DeleteGameObject(GAME_OBJECT_GROUP_TYPE eGameObjType, Actor* pGameObj)
{
	if (_pGameObjContainerList[(AkU32)eGameObjType])
	{
		List_t* pDel = _pGameObjContainerList[(AkU32)eGameObjType]->pGameObjHead;
		while (pDel != nullptr)
		{
			List_t* pNext = pDel->pNext;
			Actor* pActor = reinterpret_cast<Actor*>(pDel->pData);
			if (pGameObj == pActor)
			{
				LL_Delete(&_pGameObjContainerList[(AkU32)eGameObjType]->pGameObjHead, &_pGameObjContainerList[(AkU32)eGameObjType]->pGameObjTail, &pGameObj->tLink);
				delete pActor;
			}

			pDel = pNext;
		}
	}
}

void Scene::DeleteAllGameObject()
{
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; i++)
	{
		if (_pGameObjContainerList[i])
		{
			List_t* pDel = _pGameObjContainerList[i]->pGameObjHead;
			while (pDel != nullptr)
			{
				List_t* pNext = pDel->pNext;
				Actor* pActor = reinterpret_cast<Actor*>(pDel->pData);
				if (pActor)
				{
					delete pActor;
				}

				pDel = pNext;
			}

			FreeGameObjectContainer(_pGameObjContainerList[i]);
			_pGameObjContainerList[i] = nullptr;
		}
	}
}

void Scene::CleanUp()
{
	DeleteAllGameObject();
}

GameObjContainer_t* Scene::AllocGameObjectContainer()
{
	GameObjContainer_t* pGameObjContainer = (GameObjContainer_t*)malloc(sizeof(GameObjContainer_t));
	memset(pGameObjContainer, 0, sizeof(GameObjContainer_t));
	return pGameObjContainer;
}

void Scene::FreeGameObjectContainer(GameObjContainer_t* pGameObjContainer)
{
	if (pGameObjContainer)
	{
		free(pGameObjContainer);
	}
}
