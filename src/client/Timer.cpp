#include "pch.h"
#include "Timer.h"

/*
==================
Game Timer Class
==================
*/

UTimer::UTimer()
{
}

UTimer::~UTimer()
{
	CleanUp();
}

AkBool UTimer::Initialize()
{
	AkI64 i64CountsPerSecond = 0;
	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&i64CountsPerSecond));
	_f64SecondPerCount = 1.0 / static_cast<AkF64>(i64CountsPerSecond);

	return AK_TRUE;
}

void UTimer::Tick()
{
	if (_bStopped)
	{
		_f64DeltaTime = 0.0f;
		return;
	}

	AkI64 i64CurTime = 0;
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&i64CurTime));
	_i64CurTime = i64CurTime;

	_f64DeltaTime = (_i64CurTime - _i64PrevTime) * _f64SecondPerCount;

	_i64PrevTime = _i64CurTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (_f64DeltaTime < 0.0)
	{
		_f64DeltaTime = 0.0;
	}
}

AkF32 UTimer::GetTotalTime() const
{
	AkF32 fTotalTime = static_cast<AkF32>(((_i64CurTime - _i64PausedTime) - _i64BaseTime) * _f64SecondPerCount);

	return fTotalTime;
}

AkF32 UTimer::GetDeltaTime() const
{
	return static_cast<AkF32>(_f64DeltaTime);
}

AkF64 UTimer::GetF64TotalTime() const
{
	AkF64 f64TotalTime = static_cast<AkF64>(((_i64CurTime - _i64PausedTime) - _i64BaseTime) * _f64SecondPerCount);

	return f64TotalTime;
}

AkF64 UTimer::GetF64DeltaTime() const
{
	return _f64DeltaTime;
}

void UTimer::Reset()
{
	AkI64 i64StartTime = 0;
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&i64StartTime));

	_i64BaseTime = i64StartTime;
	_i64PrevTime = i64StartTime;
	_i64StopTime = 0;
	_bStopped = AK_FALSE;
}

void UTimer::Start()
{
	AkI64 i64StartTime = 0;
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&i64StartTime));

	if (_bStopped)
	{
		_i64PausedTime = i64StartTime - _i64StopTime;
		_i64PrevTime = i64StartTime;
		_i64StopTime = 0;
		_bStopped = AK_FALSE;
	}
}

void UTimer::Stop()
{
	if (!_bStopped)
	{
		AkI64 i64CurTime = 0;
		::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&i64CurTime));

		_i64StopTime = i64CurTime;
		_bStopped = AK_TRUE;
	}
}

void UTimer::CleanUp()
{

}
