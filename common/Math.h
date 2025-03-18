#pragma once

#include "MeshData.h"

AkI32 Clamp(const AkI32 iValue, const AkI32 iMin, const AkI32 iMax);
AkF32 Clamp(const AkF32 fValue, const AkF32 fMin, const AkF32 fMax);
AkF64 Clamp(const AkF64 dValue, const AkF64 dMin, const AkF64 dMax);

AkI32 Random(AkI32 iMin, AkI32 iMax);
AkF32 Random(AkF32 fMin, AkF32 fMax);
