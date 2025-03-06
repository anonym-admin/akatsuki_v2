#pragma once

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
	#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// Standard header
#include <Windows.h>
#include <process.h>

// DirectX
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_3.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
using namespace DirectX;

// My inc
#include "common/AkType.h"
#include "common/AkMeshData.h"
#include "common/AkEnum.h"
#include "common/AkMath.h"

#include "interface/IRenderer.h"

#include "utils/LinkedList.h"
#include "utils/HashTable.h"
#include "utils/IndexGenerator.h"
#include "utils/ProcessInfo.h"

#include "RendererType.h"