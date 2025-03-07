#pragma once

enum class COLLIDER_SHAPE_TYPE
{
	COLLIDER_SHAPE_SPHERE,
	COLLIDER_SHAPE_BOX,
	COLLIDER_SHAPE_COUNT,
};

/*
===========
Collider
===========
*/

class Actor;
class Application;

class Collider
{
public:
	static const AkU32 MAX_SPHERE_GROUP_SHAPE_COUNT = 3;

	Collider(Actor* pOwner);
	~Collider();

	AkBool Initialize(Actor* pOwner);
	void Update();
	void Render();

	AkU32 GetID() { return _uID; }
	Actor* GetOwner() { return _pOwner; }
	AkBox_t* GetBoundingBox() { return _pBox; }
	AkSphere_t* GetBoundingSphere() { return _pSphere; }
	AkTriangle_t* GetTriangle() { return _pTriangle; }
	COLLIDER_SHAPE_TYPE GetShapeType() { return _eShapeType; }

	void CreateBoundingBox(const Vector3* pMin, const Vector3* pMax);
	void CreateBoundingSphere(AkF32 fRadius, const Vector3* pCenter);
	void CreateTriangle(const Vector3* pV0, const Vector3* pV1, const Vector3* pV2);
	void DestroyBoundingBox();
	void DestroyBoundingSphere();
	void DestroyTriangle();

	virtual void OnCollision(Collider* pCollider);
	virtual void OnCollisionEnter(Collider* pCollider);
	virtual void OnCollisionExit(Collider* pCollider);

private:
	void CleanUp();

private:
	static AkU32 sm_uID;
	AkU32 _uID = 0;
	Actor* _pOwner = nullptr;
	IRenderer* _pRenderer = nullptr;
	IMeshObject* _pMeshObj = nullptr;
	Matrix _mWorldRow = Matrix();
	AkBox_t* _pBox = nullptr;
	AkSphere_t* _pSphere = nullptr;
	AkTriangle_t* _pTriangle = nullptr;
	COLLIDER_SHAPE_TYPE _eShapeType = {};

	// For Frustum culling with kd-tree.
public:
	AkBool* _pDraw = nullptr;
};
