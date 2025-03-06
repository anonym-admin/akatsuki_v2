#pragma once

/*
==========
Model
==========
*/

class UApplication;
class UAnimation;

struct ModelContext_t
{
	Matrix mWorldRow = Matrix();
	UAnimation* pAnimation = nullptr;
};

enum class MODEL_RENDER_OBJECT_TYPE
{
	BASIC_MESH_OBJ,
	SKINNED_MESH_OBJ,
};

class UModel
{
public:
	static const AkU32 MAX_MODEL_CONTEXT_NUM = 4;

	UModel();
	virtual ~UModel();

	virtual AkBool Initialize(UApplication* pApp);
	virtual void Render() = 0;
	virtual void RenderNormal() = 0;
	virtual void RenderShadow() = 0;
	virtual void Release();
	
	AkU32 AddRef();

	void CreateContextTable(Matrix* pWorldRow, UAnimation* pAnim);
	void DestroyContextTable();

	void SetModelContextTableIndex(AkU32 uModelCtxTableIndex) { _uModelCtxTableIndex = uModelCtxTableIndex; }
	void SetWorldMatrix(AkU32 uModelCtxTableIndex, const Matrix* pWorld);
	void SetRenderObject(IMeshObject* pMeshObj, MODEL_RENDER_OBJECT_TYPE eType);
	ModelContext_t* GetCurrentModelContext() { return _pModelContextTable[_uModelCtxTableIndex]; }
	const Matrix* GetWorldMatrix();
	AkU32 GetModelContextTableIndex() { return _uModelCtxTableIndex; }
	IMeshObject* GetRenderObject() { return _pMeshObj; }

	AkBool PlayAnimation(const AkF32 fDeltaTime, const wchar_t* wcClipName, AkBool bInPlace = AK_FALSE);

protected:
	AkBool CreateStaticMeshObject();
	AkBool CreateSkinnedMeshObject();
	void CreateMeshBuffersForDraw(MeshData_t* pMeshData, AkU32 uMeshDataNum);
	void DestroyMeshObject();

private:
	virtual void CleanUp();

protected:
	AkU32 _uRefCount = 0;

private:
	AkU32 _uModelCtxTableIndex = 0;
	AkU32 _uModelCtxNum = 0;
	ModelContext_t* _pModelContextTable[MAX_MODEL_CONTEXT_NUM] = {};
	IMeshObject* _pMeshObj = nullptr;
	UApplication* _pApp = nullptr;
	IRenderer* _pRenderer = nullptr;
	MODEL_RENDER_OBJECT_TYPE _eType = {};
};
