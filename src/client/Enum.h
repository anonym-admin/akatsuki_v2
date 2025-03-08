#pragma once

/*
===========
Scene Type
===========
*/

enum class SCENE_TYPE
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
	GAME_OBJ_GROUP_TYPE_MAP,
	GAME_OBJ_GROUP_TYPE_CONTAINER,
	GAME_OBJ_GROUP_TYPE_COUNT = 32,
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
	EDITOR_TYPE_MODEL,
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

enum class EVENT_TYPE
{
	CREATE_GAME_OBJECT,
	SCENE_CHANGE,
	EDITOR_CHANGE,
	SCENE_TO_EDITOR_CHANGE,
	EDITOR_TO_SCENE_CHANGE,
};

/*
============
Asset Type
============
*/

enum class ASSET_MESH_DATA_TYPE // ASSET_ANIM_TYPE 순서 동일하게.
{
	// SKINNED
	SWATGUY,
	DANCER,

	// NOT SKINNED
	BRS_74,

	COUNT,
};

enum class ASSET_TEXTURE_TYPE
{
	ENV,
	IRRADIANCE,
	SPECULAR,
	BRDF,
	DYNAMIC,

	COUNT,
};

enum class ASSET_ANIM_TYPE
{
	SWATGUY,
	DANCER,

	COUNT,
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