#pragma once

#include <stdint.h>
#include <float.h>

/*
============
Type define
============
*/

typedef int64_t AkI64;
typedef uint64_t AkU64;
typedef int32_t AkI32;
typedef uint32_t AkU32;
typedef int16_t AkI16;
typedef uint16_t AkU16;
typedef int8_t AkI8;
typedef uint8_t AkU8;
typedef float AkF32;
typedef double AkF64;
typedef float AkReal;
typedef bool AkBool;
// Int-as-bool type - has some uses for efficiency and with SIMD
typedef AkI32 AkIntBool;

#define AK_MAX_F32		3.4028234663852885981170418348452e+38F
#define AK_MAX_F64		DBL_MAX
#define AK_MAX_REAL		AX_MAX_F32

#define AK_F32_EPSILON				FLT_EPSILON
#define AK_F64_EPSILON				DBL_EPSILON
#define AK_REAL_EPSILON				AK_F32_EPSILON
#define AK_NORMALIZATION_EPSILON	(float)1e-20f

// Legacy type ranges used by PhysX
#define AK_MAX_I8	INT8_MAX
#define AK_MIN_I8	INT8_MIN
#define AK_MAX_U8	UINT8_MAX
#define AK_MIN_U8	0
#define AK_MAX_I16	INT16_MAX
#define AK_MIN_I16	INT16_MIN
#define AK_MAX_U16	UINT16_MAX
#define AK_MIN_U16	0
#define AK_MAX_I32	INT32_MAX
#define AK_MIN_I32	INT32_MIN
#define AK_MAX_U32	UINT32_MAX
#define AK_MIN_U32	0

#define AK_TRUE		true
#define AK_FALSE	false

// Light
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10


typedef AkBool(*DLL_CreateInstanceFuncPtr)(void** ppModule);
