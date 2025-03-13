#pragma once

/*
========
Model
========
*/

class Model
{
public:
	Model() = default;
	Model(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	Model(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual ~Model();

	virtual AkBool Initialize(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual AkBool Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);
	virtual void Render();
	virtual void RenderNormal();
	virtual void RenderShadow();
	void UpdateWorldRow(Matrix* pWorldRow);
	void SetWireFrame(AkBool bDrawWire);

private:
	void CleanUp();
	virtual void CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum);

protected:
	virtual void CreateMaterial(const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive);

protected:
	IMeshObject* _pMeshObj = nullptr;
	Matrix _mWorldRow = Matrix();
};

/*
===========
Model Edit
===========
*/

class ModelExporter
{
public:
	ModelExporter(const std::string& strFilaname);
	virtual ~ModelExporter();

	AkBool Initialize();

	void Load(const std::string& strFilaname);
	void ExportMaterial(const std::wstring& wcFilename);
	void ExportMesh(const std::wstring& wcFilename);

private:
	void CleanUp();

	void LoadMaterial();
	void LoadNodes(aiNode* pNode, AkI32 iIndex, AkI32 iParent);
	void LoadMeshes(aiNode* pNode);
	void LoadBones(aiMesh* pMesh, std::vector<VertexWeight_t>& vecVertexWeight);

	void SaveMaterial(const std::wstring& wcFilename);
	void SaveMesh(const std::wstring& wcFilename);
	void SaveClip(const std::wstring& wcFilename);

	std::wstring GetTextureName(aiMaterial* pMaterial, aiTextureType eTyep);

	std::wstring CreateTexture(const std::wstring& wcPath, const std::wstring& wcFileName);

private:
	Assimp::Importer* _pImporter = nullptr;
	const aiScene* _pScene = nullptr;
	
	std::vector<New_MeshData_t*> _vecMeshes = {};
	std::vector<MaterialData_t*> _vecMaterials = {};
	std::vector<NodeData_t*> _vecNodeData = {};
	std::vector<BoneData_t*> _vecBoneData = {};
	std::map<std::wstring, AkU32> _mapBone = {};

	AkI32 _iBoneCount = 0;
};

class ModelEdit
{
public:
	ModelEdit();
	virtual ~ModelEdit();

	AkBool Initialize();
	virtual void Render();

	virtual void Load();
	virtual void Save();

private:
	void CleanUp();

protected:
	IMeshObject* _pMeshObj = nullptr;
	Matrix _mWorldRow = Matrix();
};