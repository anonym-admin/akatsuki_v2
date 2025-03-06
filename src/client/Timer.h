#pragma once

/*
==================
Game Timer Class
==================
*/

class UTimer
{
public:
	UTimer();
	~UTimer();

	AkBool Initialize();
	void Tick();
	void Reset();
	void Start();
	void Stop();
	AkF32 GetTotalTime() const;
	AkF32 GetDeltaTime() const;
	AkF64 GetF64TotalTime() const;
	AkF64 GetF64DeltaTime() const;

private:
	void CleanUp();

private:
	AkI64 _i64BaseTime = 0;
	AkI64 _i64CurTime = 0;
	AkI64 _i64PrevTime = 0;
	AkI64 _i64StopTime = 0;
	AkI64 _i64PausedTime = 0;
	AkF64 _f64DeltaTime = 0.0f;
	AkF64 _f64SecondPerCount = 0.0;
	AkBool _bStopped = AK_FALSE;
};

