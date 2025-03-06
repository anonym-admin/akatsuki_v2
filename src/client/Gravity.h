#pragma once

class UActor;

class UGravity
{
public:
	UGravity();
	~UGravity();

	AkBool Initialize(UActor* pOwner);
	void Update(const AkF32 fDeltaTime);

private:
	UActor* _pOwner = nullptr;
	Vector3 _fGforce = Vector3(0.0f, -9.80665f, 0.0f); // 중력 가속도.
};

