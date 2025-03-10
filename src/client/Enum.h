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
	TREE,
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
<<<<<<< HEAD
	BULLET,
	CASING,
=======
>>>>>>> f2e096c130c4bec98199dd6ba2311eb016150af2

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