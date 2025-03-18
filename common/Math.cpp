#include "pch.h"
#include "Math.h"

AkI32 Clamp(const AkI32 iValue, const AkI32 iMin, const AkI32 iMax)
{
	if (iValue < iMin)
	{
		return iMin;
	}
	else if (iValue >= iMax)
	{
		return iMax;
	}
	else
	{
		return iValue;
	}
}

AkF32 Clamp(const AkF32 fValue, const AkF32 fMin, const AkF32 fMax)
{
	if (fValue < fMin)
	{
		return fMin;
	}
	else if (fValue >= fMax)
	{
		return fMax;
	}
	else
	{
		return fValue;
	}
}

AkF64 Clamp(const AkF64 dValue, const AkF64 dMin, const AkF64 dMax)
{
	if (dValue < dMin)
	{
		return dMin;
	}
	else if (dValue >= dMax)
	{
		return dMax;
	}
	else
	{
		return dValue;
	}
}

AkI32 Random(AkI32 iMin, AkI32 iMax)
{
    return rand() % (iMax - iMin) + iMin;
}

AkF32 Random(AkF32 fMin, AkF32 fMax)
{
    AkF32 fN = rand() / (AkF32)RAND_MAX;
    return fMin + (fMax - fMin) * fN;
}


