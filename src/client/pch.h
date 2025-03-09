#pragma once

// For Memory Debugging.
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

#include "Enum.h"
#include "Type.h"
#include "Global.h"
#include "Macro.h"
#include "FilePath.h"
#include "GameInput.h"
#include "Timer.h"
#include "GeometryGenerator.h"
#include "SceneManager.h"
#include "EditorManager.h"
#include "CollisionManager.h"
#include "EventManager.h"
#include "UIManager.h"
#include "AssetManager.h"
#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "SphereColiider.h"
#include "Transform.h"
#include "Animation.h"
#include "RigidBody.h"
#include "Controller.h"
#include "Gravity.h"
#include "SkinnedModel.h"
#include "Actor.h"

// fmod
#include "fmod.hpp"
#include "fmod_errors.h"
//// imgui
//#include <imgui.h>
//#include <imgui_impl_dx12.h>
//#include <imgui_impl_win32.h>
//#include <ImGuiFileDialog.h>
//#include <ImGuiFileDialogConfig.h>
