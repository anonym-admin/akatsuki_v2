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

class Actor;

class Scene
{
public:
	virtual ~Scene();

	virtual AkBool BeginScene() = 0;
	virtual AkBool EndScene() = 0;
	virtual void Update() = 0;
	virtual void FinalUpdate() = 0;
	virtual void RenderShadow() = 0;
	virtual void Render() = 0;
	virtual void Load(const wchar_t* wcSceneFile) = 0;

	void SetName(const wchar_t* wcName) { Name = wcName; }
	void AddGameObject(GAME_OBJECT_GROUP_TYPE eGameObjType, Actor* pGameObj);
	GameObjContainer_t* GetGroupObject(GAME_OBJECT_GROUP_TYPE eGameObjType) { return _pGameObjContainerList[(AkU32)eGameObjType]; }
	GameObjContainer_t** GetAllGameObject() { return _pGameObjContainerList; }
	AkU32 GetGameObjectNum() { return _uGameObjNum; }
	void DeleteGameObject(GAME_OBJECT_GROUP_TYPE eGameObjType, Actor* pGameObj);
	void DeleteAllGameObject();

private:
	void CleanUp();

	GameObjContainer_t* AllocGameObjectContainer();
	void FreeGameObjectContainer(GameObjContainer_t* pGameObjContainer);

private:
	// Game obj.
	GameObjContainer_t* _pGameObjContainerList[(AkU32)GAME_OBJECT_GROUP_TYPE::COUNT] = {};
	AkU32 _uGameObjNum = 0;

public:
	const wchar_t* Name = nullptr;
};

