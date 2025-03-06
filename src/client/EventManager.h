#pragma once

struct GameEventCreateGameObjectParam_t
{

};

struct GameEventSceneChangeParam_t
{
	GAME_SCENE_TYPE eBefore = {};
	GAME_SCENE_TYPE eAfter = {};
};

struct GameEventHandle_t
{
	GAME_EVENT_TYPE eEventType = {};
	void* pObj = nullptr;

	union
	{
		GameEventCreateGameObjectParam_t tCreateGameObjParam;
		GameEventSceneChangeParam_t tSceneChangeParam;
	};
};

/*
==================
GameEventManager
==================
*/

class UApplication;

class UGameEventManager
{
public:
	static const AkU32 MAX_EVENT_HANDLE_LIST_COUNT = 1024;

	UGameEventManager();
	~UGameEventManager();

	AkBool Initialize(UApplication* pApp);
	void Excute(AkF32 fDeltaTime);
	void Reset();

	void AddEvent(GameEventHandle_t* pEventHandle);

private:
	void CleanUp();

	GameEventHandle_t* Dispatch();

private:
	UApplication* _pApp = nullptr;
	GameEventHandle_t _pEventHandleList[MAX_EVENT_HANDLE_LIST_COUNT] = {};
	AkU32 _uReadPos = 0;
	AkU32 _uWritePos = 0;
};

struct EditorEventEditorChangeParam_t
{
	EDITOR_TYPE eBefore = {};
	EDITOR_TYPE eAfter = {};
};

struct EditorEventHandle_t
{
	EDITOR_EVENT_TYPE eEventType = {};
	void* pObj = nullptr;

	union
	{
		EditorEventEditorChangeParam_t tEditorChangeParam;
	};
};

/*
==================
EditorEventManager
==================
*/

class UEditorEvenetManager
{
public:
	static const AkU32 MAX_EVENT_HANDLE_LIST_COUNT = 1024;

	UEditorEvenetManager();
	~UEditorEvenetManager();

	AkBool Initialize(UApplication* pApp);
	void Excute(AkF32 fDeltaTime);
	void Reset();

	void AddEvent(EditorEventHandle_t* pEventHandle);

private:
	void CleanUp();

	EditorEventHandle_t* Dispatch();

private:
	UApplication* _pApp = nullptr;
	EditorEventHandle_t _pEventHandleList[MAX_EVENT_HANDLE_LIST_COUNT] = {};
	AkU32 _uReadPos = 0;
	AkU32 _uWritePos = 0;
};