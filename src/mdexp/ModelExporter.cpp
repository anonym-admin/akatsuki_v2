#include "ModelExporter.h"
#include "Utils.h"
#include <fstream>
#include <filesystem>

extern HWND g_hWnd;

extern string GetExtension(const string filename);

bool Write(ModelImporter& importer, std::string basePath, std::string fileName, bool isAnim)
{
	std::vector<MeshData> meshes;
	meshes = importer.m_meshes;

	ofstream fout;
	fout.open(basePath + fileName);

	// write mesh data.
	fout << "========MeshData========" << endl;
	fout << "MeshCount: " << meshes.size() << endl;
	for (int i = 0; i < meshes.size(); i++)
	{
		if (isAnim)
		{
			fout << "VertexCount: " << meshes[i].skinnedVertices.size() << "\t" << "IndexCount: " << meshes[i].indices.size() << endl;
		}
		else
		{
			fout << "VertexCount: " << meshes[i].vertices.size() << "\t" << "IndexCount: " << meshes[i].indices.size() << endl;
		}
	}
	fout << endl;

	// write material file name.
	fout << "========Material========" << endl;
	for (int i = 0; i < meshes.size(); i++)
	{
		fout << "Albedo: " << (meshes[i].albedoTextureFilename.empty() ? "Empty" : meshes[i].albedoTextureFilename) << endl;
		fout << "Emissive: " << (meshes[i].emissiveTextureFilename.empty() ? "Empty" : meshes[i].emissiveTextureFilename) << endl;
		fout << "Height: " << (meshes[i].heightTextureFilename.empty() ? "Empty" : meshes[i].heightTextureFilename) << endl;
		fout << "Normal: " << (meshes[i].normalTextureFilename.empty() ? "Empty" : meshes[i].normalTextureFilename) << endl;
		fout << "Metallic: " << (meshes[i].metallicTextureFilename.empty() ? "Empty" : meshes[i].metallicTextureFilename) << endl;
		fout << "Roughness: " << (meshes[i].roughnessTextureFilename.empty() ? "Empty" : meshes[i].roughnessTextureFilename) << endl;
		fout << "Ao: " << (meshes[i].aoTextureFilename.empty() ? "Empty" : meshes[i].aoTextureFilename) << endl;
		fout << "Opacity: " << (meshes[i].opacityTextureFilename.empty() ? "Empty" : meshes[i].opacityTextureFilename) << endl;
		fout << endl;
	}

	// write vertices and indices.
	fout << "========Vertices========" << endl;
	for (int i = 0; i < meshes.size(); i++)
	{
		if (isAnim)
		{
			for (int j = 0; j < meshes[i].skinnedVertices.size(); j++)
			{
				fout << "Position: " << meshes[i].skinnedVertices[j].position.x << " " << meshes[i].skinnedVertices[j].position.y << " " << meshes[i].skinnedVertices[j].position.z << endl;
				fout << "Normal: " << meshes[i].skinnedVertices[j].normalModel.x << " " << meshes[i].skinnedVertices[j].normalModel.y << " " << meshes[i].skinnedVertices[j].normalModel.z << endl;
				fout << "Texcoord: " << meshes[i].skinnedVertices[j].texcoord.x << " " << meshes[i].skinnedVertices[j].texcoord.y << endl;
				fout << "Tangent: " << meshes[i].skinnedVertices[j].tangentModel.x << " " << meshes[i].skinnedVertices[j].tangentModel.y << " " << meshes[i].skinnedVertices[j].tangentModel.z << endl;
				fout << "BlendWeight: ";
				for (int k = 0; k < 8; k++)
				{
					fout << meshes[i].skinnedVertices[j].blendWeights[k] << " ";
				}
				fout << endl;
				fout << "BoneIndices: ";
				for (int k = 0; k < 8; k++)
				{
					fout << (int)meshes[i].skinnedVertices[j].boneIndices[k] << " ";
				}
				fout << endl;
				fout << endl;
			}
		}
		else
		{
			for (int j = 0; j < meshes[i].vertices.size(); j++)
			{
				fout << "Position: " << meshes[i].vertices[j].position.x << " " << meshes[i].vertices[j].position.y << " " << meshes[i].vertices[j].position.z << endl;
				fout << "Normal: " << meshes[i].vertices[j].normalModel.x << " " << meshes[i].vertices[j].normalModel.y << " " << meshes[i].vertices[j].normalModel.z << endl;
				fout << "Texcoord: " << meshes[i].vertices[j].texcoord.x << " " << meshes[i].vertices[j].texcoord.y << endl;
				fout << "Tangent: " << meshes[i].vertices[j].tangentModel.x << " " << meshes[i].vertices[j].tangentModel.y << " " << meshes[i].vertices[j].tangentModel.z << endl;
				fout << endl;
			}
		}
	}
	fout << "========Indices========" << endl;
	for (int i = 0; i < meshes.size(); i++)
	{
		for (int j = 0; j < meshes[i].indices.size(); j += 3)
		{
			fout << meshes[i].indices[j] << " " << meshes[i].indices[j + 1] << " " << meshes[i].indices[j + 2] << endl;
		}
	}
	fout << endl;

	std::vector<AnimationData> animData;
	animData.push_back(importer.m_aniData);

	// write bone offset matrix
	fout << "========BoneOffsets========" << endl;
	fout << "BoneCount: " << animData[0].offsetMatrices.size() << endl;
	for (int i = 0; i < animData[0].offsetMatrices.size(); i++)
	{
		fout << "BoneOffset#" << i << endl;
		fout << animData[0].offsetMatrices[i]._11 << " " << animData[0].offsetMatrices[i]._12 << " " << animData[0].offsetMatrices[i]._13 << " " << animData[0].offsetMatrices[i]._14 << endl;
		fout << animData[0].offsetMatrices[i]._21 << " " << animData[0].offsetMatrices[i]._22 << " " << animData[0].offsetMatrices[i]._23 << " " << animData[0].offsetMatrices[i]._24 << endl;
		fout << animData[0].offsetMatrices[i]._31 << " " << animData[0].offsetMatrices[i]._32 << " " << animData[0].offsetMatrices[i]._33 << " " << animData[0].offsetMatrices[i]._34 << endl;
		fout << animData[0].offsetMatrices[i]._41 << " " << animData[0].offsetMatrices[i]._42 << " " << animData[0].offsetMatrices[i]._43 << " " << animData[0].offsetMatrices[i]._44 << endl;
		fout << endl;
	}

	// write bone hierarchy
	fout << "========BoneHierarchy========" << endl;
	for (int i = 0; i < animData[0].boneParents.size(); i++)
	{
		fout << "ParentIndexOfBone" << i << ": " << animData[0].boneParents[i] << endl;
	}
	fout << endl;

	if (fout.is_open())
	{
		fout.close();
	}

	return true;
}

bool WriteAnim(ModelImporter& importer, std::string basePath, std::string fileName)
{
	ofstream fout;
	fout.open(basePath + fileName);

	if (!fout.is_open())
	{
		return false;
	}

	std::vector<AnimationData> animData;
	animData.push_back(importer.m_aniData);

	// write animation clip
	fout << "========AnimationClip========" << endl;
	fout << "AnimationClipCount: " << 1 << endl;//animData[0].clips.size() << endl;
	for (int i = 0; i < animData[0].clips.size(); i++)
	{
		fout << "AnimationClip" << i << ": " << fileName << endl;
		fout << "Duration: " << animData[0].clips[i].duration << endl;
		fout << "TicksPerSecond: " << animData[0].clips[i].ticksPerSec << endl;
		fout << "{" << endl;
		for (int j = 0; j < animData[0].clips[i].keys.size(); j++)
		{
			fout << "\tBone" << j << " " << "KeyFrame: " << animData[0].clips[i].keys[j].size() << endl;
			fout << "\t{" << endl;
			for (int k = 0; k < animData[0].clips[i].keys[j].size(); k++)
			{
				fout << "\t\tTime: " << k << endl;
				fout << "\t\t\tPos: " << animData[0].clips[i].keys[j][k].pos.x << " " << animData[0].clips[i].keys[j][k].pos.y << " " << animData[0].clips[i].keys[j][k].pos.z << endl;
				fout << "\t\t\tScale: " << animData[0].clips[i].keys[j][k].scale.x << " " << animData[0].clips[i].keys[j][k].scale.y << " " << animData[0].clips[i].keys[j][k].scale.z << endl;
				fout << "\t\t\tQuat: " << animData[0].clips[i].keys[j][k].rot.x << " " << animData[0].clips[i].keys[j][k].rot.y << " " << animData[0].clips[i].keys[j][k].rot.z << " " << animData[0].clips[i].keys[j][k].rot.w << endl;
			}
			fout << "\t}" << endl;
		}
		fout << "}" << endl;
	}

	if (fout.is_open())
	{
		fout.close();
	}

	return true;
}

void Read(ModelImporter& importer, std::string basePath, std::string fileName)
{
	ifstream fin;
	fin.open(basePath + fileName);

	std::string buf;
	// read mesh data.
	int numMeshes = 0;
	int numVertices = 0;
	int numIndices = 0;
	fin >> buf;
	fin >> buf >> numMeshes;

	std::vector<MeshData> meshData;
	meshData.resize(numMeshes);

	for (int i = 0; i < numMeshes; i++)
	{
		fin >> buf >> numVertices >> buf >> numIndices;

		meshData[i].vertices.resize(numVertices);
		meshData[i].skinnedVertices.resize(numVertices);
		meshData[i].indices.resize(numIndices);
	}

	// read material file name.
	fin >> buf;
	for (int i = 0; i < numMeshes; i++)
	{
		fin >> buf >> meshData[i].albedoTextureFilename;
		fin >> buf >> meshData[i].emissiveTextureFilename;
		fin >> buf >> meshData[i].heightTextureFilename;
		fin >> buf >> meshData[i].normalTextureFilename;
		fin >> buf >> meshData[i].metallicTextureFilename;
		fin >> buf >> meshData[i].roughnessTextureFilename;
		fin >> buf >> meshData[i].aoTextureFilename;
		fin >> buf >> meshData[i].opacityTextureFilename;
	}

	// read vertices and indices.
	fin >> buf;
	for (int i = 0; i < numMeshes; i++)
	{
		for (int j = 0; j < meshData[i].skinnedVertices.size(); j++)
		{
			fin >> buf >> meshData[i].skinnedVertices[j].position.x >> meshData[i].skinnedVertices[j].position.y >> meshData[i].skinnedVertices[j].position.z;
			fin >> buf >> meshData[i].skinnedVertices[j].normalModel.x >> meshData[i].skinnedVertices[j].normalModel.y >> meshData[i].skinnedVertices[j].normalModel.z;
			fin >> buf >> meshData[i].skinnedVertices[j].texcoord.x >> meshData[i].skinnedVertices[j].texcoord.y;
			fin >> buf >> meshData[i].skinnedVertices[j].tangentModel.x >> meshData[i].skinnedVertices[j].tangentModel.y >> meshData[i].skinnedVertices[j].tangentModel.z;
		}
	}

	if (fin.is_open())
	{
		fin.close();
	}
}

