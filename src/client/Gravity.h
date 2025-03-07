#pragma once

class Actor;

class Gravity
{
public:
	Gravity(Actor* pOwner);
	~Gravity();

	AkBool Initialize(Actor* pOwner);
	void Update();

private:
	Actor* _pOwner = nullptr;
	Vector3 _fGforce = Vector3(0.0f, -9.80665f, 0.0f); // 중력 가속도.
};

