#pragma once

/*
========================================
������ �߻��ߴ� �浹�� �����ϱ� ���ؼ� 
�� OBJ�� �ѵ�ü ID �� ����ϱ����� �����̳�
========================================
*/

union ColliderID_u
{
	AkU64 u64ID;
	struct
	{
		AkU32 uLeftID;
		AkU32 uRightID;
	};
};

/*
=================
CollisionManager
=================
*/

class Application;
class LandScape;
class Collider;
class MapObjects;
struct KDTreeNode_t;
struct RbTreeNode_t;

class Collider;

// #include <map>

class CollisionManager
{
public:
	CollisionManager();
	~CollisionManager();

	AkBool Initialize();
	void CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE eLeft, GAME_OBJECT_GROUP_TYPE eRight);
	void AttachMap(MapObjects* pMap) { _pMap = pMap; }

	void Update();

private:
	void CleanUp();

	void CollisionStaticObjectUpdate(GAME_OBJECT_GROUP_TYPE eLeft);
	void CollisionDynamicObjectUpdate(GAME_OBJECT_GROUP_TYPE eLeft, GAME_OBJECT_GROUP_TYPE eRight);
	void CollisionLandScapeUpdate(GAME_OBJECT_GROUP_TYPE eType);
	void TraversKDTree(KDTreeNode_t* pNode, Collider* pCollider);
	AkBool IsCollision(Collider* pLeft, Collider* pRight);

private:
	Application* _pApp = nullptr;
	AkU32 _pCollisionCheckBitsTable[(AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT] = {};

	RbTreeNode_t* _pRBTree = nullptr;

	// LandScape
	LandScape* _pLandScape = nullptr;

	// Map.
	MapObjects* _pMap = nullptr;
};

