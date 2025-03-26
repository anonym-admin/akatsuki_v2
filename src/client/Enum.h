#pragma once

/*
===========
Scene Type
===========
*/

enum class SCENE_TYPE
{
	LOADING,
	INGANE,
	COMPUTE,
	COUNT,
};

/*
=======================
Game Object Type
=======================
*/

enum class GAME_OBJECT_GROUP_TYPE
{
	PLAYER,
	DANCER,
	WEAPON,
	MAP,
	CONTAINER,
	BULLET,
	CASING,
	BILLBOARD,
	TERRAIN,
	COUNT = 32,
};

/*
======================
UI Object Type
======================
*/

enum class UI_TYPE
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
	EDITOR_MODEL,
	EDITOR_MAP,
	EDITOR_PARTICLE,
	COUNT,
};

/*
================
Game Event Type
================
*/

enum class EVENT_TYPE
{
	CREATE_GAME_OBJECT,
	SCENE_CHANGE,
	EDITOR_CHANGE,
	SCENE_TO_EDITOR_CHANGE,
	EDITOR_TO_SCENE_CHANGE,
};

/*
==================
Camera Type
==================
*/

enum class CAMERA_MODE
{
	FREE,
	EDITOR,
	FOLLOW,
};


/*
================
Animation
================
*/

enum class ANIM_CLIP_STATE
{
	LOOP,
	ONCE,
	STOP,
};

enum class SWAT_ANIM_CLIP
{
	IDLE,
	WALK,
	RUN,
	
	END = 32,
};

/*
============
Collider
============
*/

enum class COLLIDER_TYPE
{
	BOX,
	SPHERE,
	CAPSULE,
	SQUARE,
	
	NONE,
};