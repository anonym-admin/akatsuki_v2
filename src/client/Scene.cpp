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

UScene::UScene()
{
}

UScene::~UScene()
{
	CleanUp();
}

AkBool UScene::Initialize(UApplication* pApp)
{
	_pApp = pApp;

	_pRenderer = _pApp->GetRenderer();

	return AK_TRUE;
}

void UScene::Update(const AkF32 fDeltaTime)
{

}

void UScene::FinalUpdate(const AkF32 fDeltaTime)
{

}

void UScene::RenderShadowPass()
{
}

void UScene::Render()
{

}

void UScene::SetName(const wchar_t* wcName)
{
	wcscpy_s(_wcName, wcName);
}

void UScene::AddGameObject(GAME_OBJECT_GROUP_TYPE eGameObjType, UActor* pGameObj)
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

void UScene::CreateCommonFontObject(const wchar_t* wcFontFamilyName, AkF32 fFontSize)
{
	_pFontObj = _pRenderer->CreateFontObject(wcFontFamilyName, fFontSize);
}

void UScene::CreateCommonSpriteObject()
{
	_pSpriteObj = _pRenderer->CreateSpriteObject();
	_pSpriteObj->SetDrawBackground(AK_FALSE);
}

void UScene::CleanUp()
{
	for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT; i++)
	{
		if (_pGameObjContainerList[i])
		{
			List_t* pDel = _pGameObjContainerList[i]->pGameObjHead;
			while (pDel != nullptr)
			{
				List_t* pNext = pDel->pNext;
				UActor* pActor = reinterpret_cast<UActor*>(pDel->pData);
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
		_pRenderer->DestroyFontObject(_pFontObj);
		_pFontObj = nullptr;
	}
	if (_pSpriteObj)
	{
		_pSpriteObj->Release();
		_pSpriteObj = nullptr;
	}
	if (_pLandScape)
	{
		delete _pLandScape;
		_pLandScape = nullptr;
	}
}

GameObjContainer_t* UScene::AllocGameObjectContainer()
{
	GameObjContainer_t* pGameObjContainer = (GameObjContainer_t*)malloc(sizeof(GameObjContainer_t));
	memset(pGameObjContainer, 0, sizeof(GameObjContainer_t));
	return pGameObjContainer;
}

void UScene::FreeGameObjectContainer(GameObjContainer_t* pGameObjContainer)
{
	if (pGameObjContainer)
	{
		free(pGameObjContainer);
	}
}
