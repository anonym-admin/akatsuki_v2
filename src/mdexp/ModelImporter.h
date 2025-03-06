#pragma once

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

#include "Type.h"
#include "AnimationClip.h"

// #define REGACY

using std::map;
using std::string;
using std::vector;

class ModelImporter
{
public:
    void Load(string basePath, string filename, std::string texturePath, bool revertNormals);
    void LoadAnimation(string basePath, string filename);

    const aiNode* FindParent(const aiNode* node);

    void ProcessNode(aiNode* node, const aiScene* scene,
        DirectX::SimpleMath::Matrix tr);

    MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);

    void ReadAnimation(const aiScene* scene);

    std::string ReadTextureFilename(const aiScene* scene, aiMaterial* material,
        aiTextureType type);

    void UpdateTangents();

    // 버텍스의 변형에 직접적으로 참여하는 뼈들의 목록을 만듭니다.
    void FindDeformingBones(const aiScene* scene);
    void UpdateBoneIDs(aiNode* node, int* counter) {
        static int id = 0;
        if (node) {
            if (m_aniData.boneNameToId.count(node->mName.C_Str())) {
                m_aniData.boneNameToId[node->mName.C_Str()] = *counter;
                *counter += 1;
            }
            for (UINT i = 0; i < node->mNumChildren; i++) {
                UpdateBoneIDs(node->mChildren[i], counter);
            }
        }
    }

public:
    string m_basePath;
    string m_texturePath;
    vector<MeshData> m_meshes;

    AnimationData m_aniData;

    bool m_isGLTF = false; // gltf or fbx
    bool m_revertNormals = false;
};

#ifdef REGACY
class ModelImporter
{
public:
	void Load(const char* basePath, const char* filename, bool revertNormals);
	void ProcessNode(aiNode* node, const aiScene* scene, Matrix tr);
	MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<MeshData> GetMeshes() { return m_meshes; }

	void PrintDebug();

private:
	const char* ReadTextureFromFilename(const aiScene* scene, aiMaterial* material, aiTextureType type);

private:
	char m_basePath[256] = {};
	std::vector<MeshData> m_meshes;
};
#endif