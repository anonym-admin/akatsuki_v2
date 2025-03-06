#pragma once

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// Standard header
#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <process.h>
#include <conio.h>

// My inc
#include "common/AkType.h"
#include "common/AkEnum.h"
#include "common/AkMeshData.h"
#include "common/AkMath.h"

#include "utils/LinkedList.h"
#include "utils/HashTable.h"
#include "utils/UtilFunc.h"
#include "utils/RedBlackTree.h"
#include "utils/Queue.h"

#include "interface/IRenderer.h"

#include "Type.h"

// fmod
#include "fmod.hpp"
#include "fmod_errors.h"
