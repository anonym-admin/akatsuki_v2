#include "pch.h"
#include "Scene.h"
#include "Actor.h"
#include "Application.h"
#include "LandScape.h"

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

void Scene::RenderShadow()
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
				pActor->RenderShadow();

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

void Scene::CreateCommonFontObject(const wchar_t* wcFontFamilyName, AkF32 fFontSize)
{
	_pFontObj = GRenderer->CreateFontObject(wcFontFamilyName, fFontSize);
}

void Scene::CreateCommonSpriteObject()
{
	_pSpriteObj = GRenderer->CreateSpriteObject();
	_pSpriteObj->SetDrawBackground(AK_FALSE);
}

void Scene::CleanUp()
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

	if (_pFontObj)
	{
		GRenderer->DestroyFontObject(_pFontObj);
		_pFontObj = nullptr;
	}
	if (_pSpriteObj)
	{
		_pSpriteObj->Release();
		_pSpriteObj = nullptr;
	}
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
