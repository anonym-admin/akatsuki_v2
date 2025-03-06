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

class UApplication;
class ULandScape;
class UCollider;
class UWorldMapContainer;
struct KDTreeNode_t;
struct RbTreeNode_t;

// #include <map>

class UCollisionManager
{
public:
	UCollisionManager();
	~UCollisionManager();

	AkBool Initialize(UApplication* pApp);
	void CollisionGroupCheck(GAME_OBJECT_GROUP_TYPE eLeft, GAME_OBJECT_GROUP_TYPE eRight);
	void AttachMap(UWorldMapContainer* pMap) { _pMap = pMap; }

	void Update();

private:
	void CleanUp();

	void CollisionStaticObjectUpdate(GAME_OBJECT_GROUP_TYPE eLeft);
	void CollisionDynamicObjectUpdate(GAME_OBJECT_GROUP_TYPE eLeft, GAME_OBJECT_GROUP_TYPE eRight);
	void CollisionLandScapeUpdate(GAME_OBJECT_GROUP_TYPE eType);
	void TraversKDTree(KDTreeNode_t* pNode, UCollider* pCollider);
	AkBool IsCollision(UCollider* pLeft, UCollider* pRight);

private:
	UApplication* _pApp = nullptr;
	AkU32 _pCollisionCheckBitsTable[(AkU32)GAME_OBJECT_GROUP_TYPE::GAME_OBJ_GROUP_TYPE_COUNT] = {};

	RbTreeNode_t* _pRBTree = nullptr;

	// LandScape
	ULandScape* _pLandScape = nullptr;

	// Map.
	UWorldMapContainer* _pMap = nullptr;
};

