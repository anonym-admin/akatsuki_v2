#pragma once

/*
===========
Collider
===========
*/

class Transform;
class BoxCollider;
class SphereCollider;
class CapsuleCollider;
class SquareCollider;
class Actor;

class Collider
{
public:
	static AkBool DRAW_COLLIDER;

	Collider(Actor* pOwner);
	virtual ~Collider();

	AkBool Initialize(Actor* pOwner);
	AkBool Intersect(Collider* pCollider);

	virtual AkBool RayIntersect(DirectX::SimpleMath::Ray tRay, Vector3* pOutHitPos = nullptr, AkF32* pOutDist = nullptr) = 0;
	virtual AkBool BoxIntersect(BoxCollider* pCollider) = 0;
	virtual AkBool SphereIntersect(SphereCollider* pCollider) = 0;
	virtual AkBool CapsuleIntersect(CapsuleCollider* pCapsule) = 0;
	virtual AkBool SqaureIntersect() { return AK_TRUE; }

	virtual void OnCollisionEnter(Collider* pCollider) = 0;
	virtual void OnCollision(Collider* pCollider) = 0;
	virtual void OnCollisionExit(Collider* pCollider) = 0;

	virtual AkF32 Radius() = 0;

	void Update();
	void Render();

	Transform* GetTransform() { return _pTransform; }
	Actor* GetOwner() { return _pOwner; }
	AkI32 GetID() { return _iID; }
	void SetColor(const Vector3* pColor);

private:
	void CleanUp();

protected:
	Actor* _pOwner = nullptr;
	COLLIDER_TYPE _eType = COLLIDER_TYPE::NONE;
	ILineObject* _pLineObj = nullptr;
	Transform* _pTransform = nullptr;

private:
	AkI32 _iID = -1;
	static AkU32 ID;
};

