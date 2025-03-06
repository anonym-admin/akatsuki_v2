#pragma once

/*
===================
Geometry Generator
===================
*/

class UAnimation;
class UApplication;

class UGeometryGenerator
{
public:
	static MeshData_t* MakeTriangle(AkU32* pMeshDataNum);
	static MeshData_t* MakeSquare(AkU32* pMeshDataNum, const AkF32 fScale = 1.0f, const Vector2* pTexScale = nullptr);
	static MeshData_t* MakeSphere(AkU32* pMeshDataNum, const AkF32 uRadius, const AkU32 uNumSlices, const AkU32 uNumStacks, const Vector2* pTexScale = nullptr);
	static MeshData_t* MakeCube(AkU32* pMeshDataNum, const AkF32 fScale = 1.0f);
	static MeshData_t* MakeCubeWidthExtent(AkU32* pMeshDataNum, const Vector3* pExtent);
	static MeshData_t* MakeWireCube(AkU32* pMeshDataNum, const AkF32 fScale = 1.0f);
	static MeshData_t* MakeWireCubeWidthExtent(AkU32* pMeshDataNum, Vector3* vP0, Vector3* vP1, Vector3* vP2, Vector3* vP3, Vector3* vP4, Vector3* vP5, Vector3* vP6, Vector3* vP7);
	static MeshData_t* MakeWireSphere(AkU32* pMeshDataNum, const AkF32 fScale = 1.0f);
	static MeshData_t* MakeGrid(AkU32* pMeshDataNum, const AkF32 fScale, const AkU32 uNumSlices, const AkU32 uNumStacks, const Vector2* pTexScale = nullptr);
	static MeshData_t* ReadFromFile(UApplication* pApp, AkU32* pMeshDataNum, const wchar_t* wcBasePath, const wchar_t* wcFilename, AkBool bIsAnim = false, Matrix* pDefaultMat = nullptr, Matrix const** pBoneOffsetMat = nullptr, AkI32 const** pBoneHierarchy = nullptr, AkU32* pBoneNum = nullptr);
	static void NormalizeMeshData(MeshData_t* pMeshData, AkU32 uMeshDataNum, AkF32 fScaleLength);
	static void NormalizeMeshData(MeshData_t* pMeshData, AkU32 uMeshDataNum, const AkF32 fScaleLength, AkBool bIsAnim , Matrix* pDefaultMatrix);

	static void DestroyGeometry(MeshData_t* pMeshData, AkU32 uMeshDataNum);
};

