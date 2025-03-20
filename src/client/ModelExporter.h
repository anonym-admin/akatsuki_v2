#pragma once

/*
===============
Model Exporter
===============
*/

class ModelExporter
{
public:
	ModelExporter(const std::string& strFilaname);
	~ModelExporter();

	AkBool Initialize();

	void Load(const std::string& strFilaname);
	const aiNode* FindParent(const aiNode* node);
	void ProcessNode(aiNode* node, const aiScene* scene, DirectX::SimpleMath::Matrix tr);
	ModelMeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);
	void ReadAnimation(const aiScene* scene);
	std::string ReadTextureFilename(const aiScene* scene, aiMaterial* material, aiTextureType type);
	void UpdateTangents();
	void FindDeformingBones(const aiScene* scene);
	void UpdateBoneIDs(aiNode* node, int* counter);

	void ExportMesh();
	void ExportClip();

private:
	void CleanUp();

	void SaveMesh(const std::wstring& wcPath);
	void SaveClip(const std::wstring& wcPath);

private:
	Assimp::Importer* _pImporter = nullptr;
	const aiScene* _pScene = nullptr;

	std::vector<ModelMeshData> _vecModelMeshData = {};
	AnimationData _tAnimData = {};
	AkBool _bIsGLTF = AK_FALSE;
	AkBool _bRevertNormal = AK_FALSE;

	std::wstring _wcFilename = L"";
	std::wstring _wcFolder = L"";
};

