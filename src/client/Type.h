#pragma once

/*
===========
Scene Type
===========
*/

enum class GAME_SCENE_TYPE
{
	SCENE_TYPE_LOADING,
	SCENE_TYPE_INGANE,
	SCENE_TYPE_COUNT,
};

/*
=======================
Game Object Type
=======================
*/

enum class GAME_OBJECT_GROUP_TYPE
{
	GAME_OBJ_GROUP_TYPE_PLAYER,
	GAME_OBJ_GROUP_TYPE_DANCER,
	GAME_OBJ_GROUP_TYPE_WEAPON,
	GAME_OBJ_GROUP_TYPE_MAP, // 움직이지 않는 오브젝트
	GAME_OBJ_GROUP_TYPE_COUNT = 32,
};

/*
======================
UI Object Type
======================
*/

enum class UI_OBJECT_TYPE
{
	UI_OBJ_SYS_INFO_TEXT,
	UI_OBJ_CHAT_INPUT_TEXT,
	UI_OBJ_TEST_STATIC_TEXT,
	UI_OBJ_EXIT,
	UI_OBJ_TYPE_COUNT = 32,
};

/*
============
Editor Type
============
*/

enum class EDITOR_TYPE
{
	EDITOR_TYPE_CHARACTER,
	EDITOR_TYPE_MAP,
	EDITOR_TYPE_COUNT,
};

/*
================
Collider Type
================
*/

enum class GAME_COLLIDER_TYPE
{
	GAME_COLLIDER_TYPE_SPHERE,
	GAME_COLLIDER_TYPE_BOX,
	GAME_COLLIDER_TYPE_PLANE,
	GAME_COLLIDER_TYPE_TRI_LIST,
	GAME_COLLIDER_TYPE_SPHERE_GROUP,
};

/*
================
Game Event Type
================
*/

enum class GAME_EVENT_TYPE
{
	GAME_EVENT_TYPE_CREATE_GAME_OBJECT,
	GAME_EVENT_TYPE_SCENE_CHANGE,
};

/*
==================
Editor Event Type
==================
*/

enum class EDITOR_EVENT_TYPE
{
	EDITOR_EVENT_TYPE_CREATE_GAME_OBJECT,
	EDITOR_EVENT_TYPE_EDITOR_CHANGE,
};