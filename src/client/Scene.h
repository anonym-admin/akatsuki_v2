#pragma once

/*
==================
Scene base class
==================
*/

struct GameObjContainer_t
{
	List_t* pGameObjHead = nullptr;
	List_t* pGameObjTail = nullptr;
};

class UApplication;
class UActor;
class ULandScape;

class UScene
{
public:
	UScene();
	virtual ~UScene();

	virtual AkBool Initialize(UApplication* pApp) = 0;
	virtual AkBool BeginScene() = 0;
	virtual AkBool EndScene() = 0;
	virtual void Update(const AkF32 fDeltaTime);
	virtual void FinalUpdate(const AkF32 fDeltaTime);
	virtual void RenderShadowPass();
	virtual void Render();

	void SetName(const wchar_t* wcName);
	void AddGameObject(GAME_OBJECT_GROUP_TYPE eGameObjType, UActor* pGameObj);
	GameObjContainer_t* GetGroupObject(GAME_OBJECT_GROUP_TYPE eGameObjType) { return _pGameObjContainerList[(AkU32)eGameObjType]; }
	GameObjContainer_t** GetAllGameObject() { return _pGameObjContainerList; }
	AkU32 GetGameObjectNum() { return _uGameObjNum; }
	void* GetCommonFontObject() { return _pFontObj; }
	ISpriteObject* GetCommonSpriteObject() { return _pSpriteObj; }
	UApplication* GetApp() { return _pApp; }
	IRenderer* GetRenderer() { return _pRenderer; }
	ULandScape* GetLandScape() { return _pLandScape; }

protected:
	void CreateCommonFontObject(const wchar_t* wcFontFamilyName, AkF32 fFontSize);
	void CreateCommonSpriteObject();

private:
	void CleanUp();

	GameObjContainer_t* AllocGameObjectContainer();
	void FreeGameObjectContainer(GameObjContainer_t* pGameObjContainer);

private:
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;

	// Common sprite obj.
	void* _pFontObj = nullptr;
	ISpriteObject* _pSpriteObj = nullptr;

	// Game obj.
	GameObjContainer_t* _pGameObjContainerList[(AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT] = {};
	AkU32 _uGameObjNum = 0;
	wchar_t _wcName[32] = {};

protected:
	// Land scape.
	ULandScape* _pLandScape = nullptr;
};

