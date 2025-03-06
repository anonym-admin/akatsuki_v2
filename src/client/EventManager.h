#pragma once

struct EventCreateGameObjectParam_t
{

};

struct EventSceneChangeParam_t
{
	SCENE_TYPE eBefore = {};
	SCENE_TYPE eAfter = {};
};

struct EventHandle_t
{
	GAME_EVENT_TYPE eEventType = {};
	void* pObj = nullptr;

	union
	{
		EventCreateGameObjectParam_t tCreateGameObjParam;
		EventSceneChangeParam_t tSceneChangeParam;
	};
};

/*
==================
EventManager
==================
*/

class Application;

class EventManager
{
public:
	static const AkU32 MAX_EVENT_HANDLE_LIST_COUNT = 1024;

	~EventManager();

	void Excute(AkF32 fDeltaTime);
	void Reset();

	void AddEvent(EventHandle_t* pEventHandle);

private:
	void CleanUp();

	EventHandle_t* Dispatch();

private:
	Application* _pApp = nullptr;
	EventHandle_t _pEventHandleList[MAX_EVENT_HANDLE_LIST_COUNT] = {};
	AkU32 _uReadPos = 0;
	AkU32 _uWritePos = 0;
};

