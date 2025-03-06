#include "ModelImporter.h"
#include "Utils.h"
#include <DirectXMesh.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

using namespace std;
using namespace DirectX::SimpleMath;

void UpdateNormals(vector<MeshData>& meshes) {

	// ��� ���Ͱ� ���� ��츦 ����Ͽ� �ٽ� ���
	// �� ��ġ���� �� ���ؽ��� �־�� ���� ���踦 ã�� �� ����

	// DirectXMesh�� ComputeNormals()�� ����մϴ�.
	// https://github.com/microsoft/DirectXMesh/wiki/ComputeNormals

	for (auto& m : meshes) {

		vector<Vector3> normalsTemp(m.vertices.size(), Vector3(0.0f));
		vector<float> weightsTemp(m.vertices.size(), 0.0f);

		for (int i = 0; i < m.indices.size(); i += 3) {

			int idx0 = m.indices[i];
			int idx1 = m.indices[i + 1];
			int idx2 = m.indices[i + 2];

			auto v0 = m.vertices[idx0];
			auto v1 = m.vertices[idx1];
			auto v2 = m.vertices[idx2];

			auto faceNormal =
				(v1.position - v0.position).Cross(v2.position - v0.position);

			normalsTemp[idx0] += faceNormal;
			normalsTemp[idx1] += faceNormal;
			normalsTemp[idx2] += faceNormal;
			weightsTemp[idx0] += 1.0f;
			weightsTemp[idx1] += 1.0f;
			weightsTemp[idx2] += 1.0f;
		}

		for (int i = 0; i < m.vertices.size(); i++) {
			if (weightsTemp[i] > 0.0f) {
				m.vertices[i].normalModel = normalsTemp[i] / weightsTemp[i];
				m.vertices[i].normalModel.Normalize();
			}
		}
	}
}

string GetExtension(const string filename) {
	string ext(filesystem::path(filename).extension().string());
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	return ext;
}

void ModelImporter::ReadAnimation(const aiScene* pScene) {

	m_aniData.clips.resize(pScene->mNumAnimations);

	for (uint32_t i = 0; i < pScene->mNumAnimations; i++) {

		auto& clip = m_aniData.clips[i];

		const aiAnimation* ani = pScene->mAnimations[i];

		clip.duration = ani->mDuration;
		clip.ticksPerSec = ani->mTicksPerSecond;
		clip.keys.resize(m_aniData.boneNameToId.size());
		clip.numChannels = ani->mNumChannels;

		for (uint32_t c = 0; c < ani->mNumChannels; c++) {
			const aiNodeAnim* nodeAnim = ani->mChannels[c];
			const int boneId =
				m_aniData.boneNameToId[nodeAnim->mNodeName.C_Str()];
			clip.keys[boneId].resize(nodeAnim->mNumPositionKeys);
			for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++) {
				const auto pos = nodeAnim->mPositionKeys[k].mValue;
				const auto rot = nodeAnim->mRotationKeys[k].mValue;
				const auto scale = nodeAnim->mScalingKeys[k].mValue;

				//float time0 = nodeAnim->mPositionKeys[k].mTime;
				//float time1 = nodeAnim->mPositionKeys[k].mTime;
				//float time2 = nodeAnim->mPositionKeys[k].mTime;
				// std::cout << time0 << " " << time1 << " " << time2 << endl;

				auto& key = clip.keys[boneId][k];
				key.pos = { pos.x, pos.y, pos.z };
				key.rot = Quaternion(rot.x, rot.y, rot.z, rot.w);
				key.scale = { scale.x, scale.y, scale.z };
			}
		}
	}
}

/*
 * �������� ������ �ְ� Ʈ�� ������
 * �� �߿��� Vertex�� ������ �ִ� �͵��� �Ϻ���
 * Vertex�� ������ �ִ� ����� �θ����� �����ؼ�
 * Ʈ������ ������ ����
 */
void ModelImporter::Load(std::string basePath, std::string filename, std::string texturePath, bool revertNormals)
{

	if (GetExtension(filename) == ".gltf") {
		m_isGLTF = true;
		m_revertNormals = revertNormals;
	}

	m_basePath = basePath; // �ؽ��� �о���� �� �ʿ�
	m_texturePath = texturePath;

	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(
		m_basePath + filename,
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	// ReadFile()���� ��쿡 ���� �������� �ɼǵ� ���� ����
	// aiProcess_JoinIdenticalVertices | aiProcess_PopulateArmatureData |
	// aiProcess_SplitByBoneCount |
	// aiProcess_Debone); // aiProcess_LimitBoneWeights

	if (pScene) {

		// 1. ��� �޽��� ���ؼ� ���ؽ��� ������ �ִ� ������ ����� �����.
		FindDeformingBones(pScene);

		// 2. Ʈ�� ������ ���� ������Ʈ ������� ������ �ε����� �����Ѵ�
		int counter = 0;
		UpdateBoneIDs(pScene->mRootNode, &counter);

		// 3. ������Ʈ ������� �� �̸� ���� (boneIdToName)
		m_aniData.boneIdToName.resize(m_aniData.boneNameToId.size());
		for (auto& i : m_aniData.boneNameToId)
			m_aniData.boneIdToName[i.second] = i.first;

		// ������
		// cout << "Num boneNameToId : " << m_aniData.boneNameToId.size() <<
		// endl; for (auto &i : m_aniData.boneNameToId) {
		//    cout << "NameId pair : " << i.first << " " << i.second << endl;
		//}
		// cout << "Num boneIdToName : " << m_aniData.boneIdToName.size() <<
		// endl; for (size_t i = 0; i < m_aniData.boneIdToName.size(); i++) {
		//    cout << "BoneId: " << i << " " << m_aniData.boneIdToName[i] <<
		//    endl;
		//}
		// exit(-1);

		// �� ���� �θ� �ε����� ������ �غ�
		m_aniData.boneParents.resize(m_aniData.boneNameToId.size(), -1);

		Matrix tr; // Initial transformation
		ProcessNode(pScene->mRootNode, pScene, tr);

		// ������
		// cout << "Num boneIdToName : " << m_aniData.boneIdToName.size() <<
		// endl; for (size_t i = 0; i < m_aniData.boneIdToName.size(); i++) {
		//    cout << "BoneId: " << i << " " << m_aniData.boneIdToName[i]
		//         << " , Parent: "
		//         << (m_aniData.boneParents[i] == -1
		//                 ? "NONE"
		//                 : m_aniData.boneIdToName[m_aniData.boneParents[i]])
		//         << endl;
		//}

		// �ִϸ��̼� ���� �б�
		if (pScene->HasAnimations())
			ReadAnimation(pScene);

		// UpdateNormals(this->meshes); // Vertex Normal�� ���� ��� (�����)

		UpdateTangents();
	}
	else {
		std::cout << "Failed to read file: " << m_basePath + filename
			<< std::endl;
		auto errorDescription = importer.GetErrorString();
		std::cout << "Assimp error: " << errorDescription << endl;
	}
}

void ModelImporter::LoadAnimation(string basePath, string filename) {

	m_basePath = basePath;

	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(
		m_basePath + filename,
		aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (pScene && pScene->HasAnimations()) {
		ReadAnimation(pScene);
	}
	else {
		std::cout << "Failed to read animation from file: "
			<< m_basePath + filename << std::endl;
		auto errorDescription = importer.GetErrorString();
		std::cout << "Assimp error: " << errorDescription << endl;
	}
}

void ModelImporter::UpdateTangents() {

	using namespace std;
	using namespace DirectX;

	// https://github.com/microsoft/DirectXMesh/wiki/ComputeTangentFrame

	for (auto& m : this->m_meshes) {

		vector<XMFLOAT3> positions(m.vertices.size());
		vector<XMFLOAT3> normals(m.vertices.size());
		vector<XMFLOAT2> texcoords(m.vertices.size());
		vector<XMFLOAT3> tangents(m.vertices.size());
		vector<XMFLOAT3> bitangents(m.vertices.size());

		for (size_t i = 0; i < m.vertices.size(); i++) {
			auto& v = m.vertices[i];
			positions[i] = v.position;
			normals[i] = v.normalModel;
			texcoords[i] = v.texcoord;
		}

		ComputeTangentFrame(m.indices.data(), m.indices.size() / 3,
			positions.data(), normals.data(), texcoords.data(),
			m.vertices.size(), tangents.data(),
			bitangents.data());

		for (size_t i = 0; i < m.vertices.size(); i++) {
			m.vertices[i].tangentModel = tangents[i];
		}

		if (m.skinnedVertices.size() > 0) {
			for (size_t i = 0; i < m.skinnedVertices.size(); i++) {
				m.skinnedVertices[i].tangentModel = tangents[i];
			}
		}
	}
}

// ���ؽ��� ������ ���������� �����ϴ� ������ ����� ����ϴ�.

void ModelImporter::FindDeformingBones(const aiScene* scene) {
	for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
		const auto* mesh = scene->mMeshes[i];
		if (mesh->HasBones()) {
			for (uint32_t i = 0; i < mesh->mNumBones; i++) {
				const aiBone* bone = mesh->mBones[i];

				// bone�� �����Ǵ� node�� �̸��� ����
				// �ڿ��� node �̸����� �θ� ã�� �� ����
				m_aniData.boneNameToId[bone->mName.C_Str()] = -1;

				// ����: ���� ������ ������Ʈ ������ �ƴ�

				// ��Ÿ: bone->mWeights == 0�� ��쿡�� ���Խ�����
				// ��Ÿ: bone->mNode = 0�̶� ��� �Ұ�
			}
		}
	}
}

// �������� ���� ������ �ǳʶٰ� �θ� ��� ã��
const aiNode* ModelImporter::FindParent(const aiNode* node) {
	if (!node)
		return nullptr;
	if (m_aniData.boneNameToId.count(node->mName.C_Str()) > 0)
		return node;
	return FindParent(node->mParent);
}

void ModelImporter::ProcessNode(aiNode* node, const aiScene* scene, Matrix tr) {

	// https://ogldev.org/www/tutorial38/tutorial38.html
	// If a node represents a bone in the hierarchy then the node name must
	// match the bone name.

	// ���Ǵ� �θ� ���� ã�Ƽ� �θ��� �ε��� ����
	if (node->mParent && m_aniData.boneNameToId.count(node->mName.C_Str()) &&
		FindParent(node->mParent)) {
		const auto boneId = m_aniData.boneNameToId[node->mName.C_Str()];
		m_aniData.boneParents[boneId] =
			m_aniData.boneNameToId[FindParent(node->mParent)->mName.C_Str()];
	}

	Matrix m(&node->mTransformation.a1);
	m = m.Transpose() * tr;

	for (UINT i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		auto newMesh = this->ProcessMesh(mesh, scene);
		for (auto& v : newMesh.vertices) {
			v.position = DirectX::SimpleMath::Vector3::Transform(v.position, m);
		}
		m_meshes.push_back(newMesh);
	}

	for (UINT i = 0; i < node->mNumChildren; i++) {
		this->ProcessNode(node->mChildren[i], scene, m);
	}
}

string RemoveExtension(const std::string& filename) {
	size_t lastDot = filename.find_last_of(".");
	if (lastDot == std::string::npos) {
		return filename; // Ȯ���ڰ� ���� ��� ���� ���� �̸� ��ȯ
	}
	return filename.substr(0, lastDot); // Ȯ���� ����
}

string ModelImporter::ReadTextureFilename(const aiScene* scene,
	aiMaterial* material,
	aiTextureType type) {

	if (material->GetTextureCount(type) > 0) {
		aiString filepath;
		material->GetTexture(type, 0, &filepath);

		string fullPath =
			m_texturePath +
			string(filesystem::path(filepath.C_Str()).filename().string());

		// 1. ������ ������ �����ϴ��� Ȯ��
		if (!filesystem::exists(fullPath)) {
			// 2. ������ ���� ��� Ȥ�� fbx ��ü�� Embedded���� Ȯ��
			const aiTexture* texture =
				scene->GetEmbeddedTexture(filepath.C_Str());
			if (texture) {
				// 3. Embedded texture�� �����ϰ� png�� ��� ����
				if (string(texture->achFormatHint).find("png") !=
					string::npos) {

					string texPath = m_texturePath + string(filesystem::path(filepath.C_Str()).filename().string());

					ofstream fs(texPath.c_str(), ios::binary | ios::out);
					fs.write((char*)texture->pcData, texture->mWidth);
					fs.close();
					// ����: compressed format�� ��� texture->mHeight�� 0

					texPath = RemoveExtension(texPath);

					wstring convFilePath = L"";
					convFilePath.assign(texPath.begin(), texPath.end());

					// Convert PNG to DDS.
					PNG_To_DDS(convFilePath.c_str());

					// Delete PNG File.
					system("dir");

					wchar_t wcCurDir[MAX_PATH] = {};
					GetCurrentDirectory(MAX_PATH, wcCurDir);

					SetCurrentDirectory(L"C:/Users/skdfw/My/Project/akatsuki/assets/model/");

					string delPath = string("del " + string(filesystem::path(filepath.C_Str()).filename().string())).c_str();

					system(delPath.c_str());

					SetCurrentDirectory(wcCurDir);
				}
			}
			else {
				cout << fullPath << " doesn't exists. Return empty filename."
					<< endl;
			}
		}
		else {
			return fullPath;
		}

		return fullPath;

	}
	else {
		return "";
	}
}

MeshData ModelImporter::ProcessMesh(aiMesh* mesh, const aiScene* scene) {

	MeshData newMesh;
	auto& vertices = newMesh.vertices;
	auto& indices = newMesh.indices;
	auto& skinnedVertices = newMesh.skinnedVertices;

	// Walk through each of the mesh's vertices
	for (UINT i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		vertex.normalModel.x = mesh->mNormals[i].x;
		if (m_isGLTF) {
			vertex.normalModel.y = mesh->mNormals[i].z;
			vertex.normalModel.z = -mesh->mNormals[i].y;
		}
		else {
			vertex.normalModel.y = mesh->mNormals[i].y;
			vertex.normalModel.z = mesh->mNormals[i].z;
		}

		if (m_revertNormals) {
			vertex.normalModel *= -1.0f;
		}

		vertex.normalModel.Normalize();

		if (mesh->mTextureCoords[0]) {
			vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->HasBones()) {

		vector<vector<float>> boneWeights(vertices.size());
		vector<vector<uint8_t>> boneIndices(vertices.size());

		m_aniData.offsetMatrices.resize(m_aniData.boneNameToId.size());
		m_aniData.boneTransforms.resize(m_aniData.boneNameToId.size());

		int count = 0;
		for (uint32_t i = 0; i < mesh->mNumBones; i++) {
			const aiBone* bone = mesh->mBones[i];

			// �����
			// cout << "BoneMap " << count++ << " : " << bone->mName.C_Str()
			//     << " NumBoneWeights = " << bone->mNumWeights << endl;

			const uint32_t boneId = m_aniData.boneNameToId[bone->mName.C_Str()];

			m_aniData.offsetMatrices[boneId] =
				Matrix((float*)&bone->mOffsetMatrix).Transpose();

			// �� ���� ������ �ִ� Vertex�� ����
			for (uint32_t j = 0; j < bone->mNumWeights; j++) {
				aiVertexWeight weight = bone->mWeights[j];
				assert(weight.mVertexId < boneIndices.size());
				boneIndices[weight.mVertexId].push_back(boneId);
				boneWeights[weight.mVertexId].push_back(weight.mWeight);
			}
		}

		// �������� Vertex �ϳ��� ������ �ִ� Bone�� �ִ� 4��
		// ������ �� ���� ���� �ִµ� �𵨸� ����Ʈ����� �����ϰų�
		// �о���̸鼭 weight�� �ʹ� ���� �͵��� �� ���� ����

		int maxBones = 0;
		for (int i = 0; i < boneWeights.size(); i++) {
			maxBones = std::max(maxBones, int(boneWeights[i].size()));
		}

		cout << "Max number of influencing bones per vertex = " << maxBones
			<< endl;

		skinnedVertices.resize(vertices.size());
		for (int i = 0; i < vertices.size(); i++) {
			skinnedVertices[i].position = vertices[i].position;
			skinnedVertices[i].normalModel = vertices[i].normalModel;
			skinnedVertices[i].texcoord = vertices[i].texcoord;

			for (int j = 0; j < boneWeights[i].size(); j++) {
				skinnedVertices[i].blendWeights[j] = boneWeights[i][j];
				skinnedVertices[i].boneIndices[j] = boneIndices[i][j];
			}
		}

		// ������ ��� (boneWeights)
		// for (int i = 0; i < boneWeights.size(); i++) {
		//    cout << boneWeights[i].size() << " : ";
		//    for (int j = 0; j < boneWeights[i].size(); j++) {
		//        cout << boneWeights[i][j] << " ";
		//    }
		//    cout << " | ";
		//    for (int j = 0; j < boneIndices[i].size(); j++) {
		//        cout << int(boneIndices[i][j]) << " ";
		//    }
		//    cout << endl;
		//}
	}

	// http://assimp.sourceforge.net/lib_html/materials.html
	if (mesh->mMaterialIndex >= 0) {

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		newMesh.albedoTextureFilename =
			ReadTextureFilename(scene, material, aiTextureType_BASE_COLOR);
		if (newMesh.albedoTextureFilename.empty()) {
			newMesh.albedoTextureFilename =
				ReadTextureFilename(scene, material, aiTextureType_DIFFUSE);
		}
		newMesh.emissiveTextureFilename =
			ReadTextureFilename(scene, material, aiTextureType_EMISSIVE);
		newMesh.heightTextureFilename =
			ReadTextureFilename(scene, material, aiTextureType_HEIGHT);
		newMesh.normalTextureFilename =
			ReadTextureFilename(scene, material, aiTextureType_NORMALS);
		newMesh.metallicTextureFilename =
			ReadTextureFilename(scene, material, aiTextureType_METALNESS);
		newMesh.roughnessTextureFilename = ReadTextureFilename(
			scene, material, aiTextureType_DIFFUSE_ROUGHNESS);
		newMesh.aoTextureFilename = ReadTextureFilename(
			scene, material, aiTextureType_AMBIENT_OCCLUSION);
		if (newMesh.aoTextureFilename.empty()) {
			newMesh.aoTextureFilename =
				ReadTextureFilename(scene, material, aiTextureType_LIGHTMAP);
		}
		newMesh.opacityTextureFilename =
			ReadTextureFilename(scene, material, aiTextureType_OPACITY);

		if (!newMesh.opacityTextureFilename.empty()) {
			cout << newMesh.albedoTextureFilename << endl;
			cout << "Opacity " << newMesh.opacityTextureFilename << endl;
		}

		// ������
		// for (size_t i = 0; i < 22; i++) {
		//    cout << i << " " << ReadTextureFilename(material,
		//    aiTextureType(i))
		//         << endl;
		//}
	}

	return newMesh;
}

#ifdef REGACY
void ModelImporter::Load(const char* basePath, const char* filename, bool revertNormals)
{
	Assimp::Importer importer;

	char filepath[256] = {};
	strcat_s(m_basePath, 256, basePath);
	strcat_s(filepath, 256, basePath);
	strcat_s(filepath, 256, filename);
	const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (scene)
	{
		Matrix tr;
		ProcessNode(scene->mRootNode, scene, tr);
	}
	else
	{
		printf("Failed to read file: %s\n", filepath);
		printf("Assimp error: %s\n", importer.GetErrorString());
	}
}

void ModelImporter::ProcessNode(aiNode* node, const aiScene* scene, Matrix tr)
{
	Matrix m(&node->mTransformation.a1);
	m = m.Transpose() * tr;

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		MeshData newMesh = this->ProcessMesh(mesh, scene);
		for (auto& v : newMesh.vertices)
		{
			v.position = Vector3::Transform(v.position, m);
		}
		m_meshes.push_back(newMesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		this->ProcessNode(node->mChildren[i], scene, tr);
	}
}

const char* ModelImporter::ReadTextureFromFilename(const aiScene* scene, aiMaterial* material, aiTextureType type)
{
	char* ret = new char[256];

	if (material->GetTextureCount(type) > 0)
	{
		aiString filePath;
		material->GetTexture(type, 0, &filePath);

		char fileName[256] = {};
		GetFileName(fileName, filePath.C_Str());

		char fullPath[256] = {};
		strcat_s(fullPath, 256, m_basePath);
		strcat_s(fullPath, 256, fileName);

		memcpy(ret, fullPath, 256);

		if (!PathFileExistsA(fullPath))
		{
			const aiTexture* texture = scene->GetEmbeddedTexture(filePath.C_Str());
			if (texture)
			{
				char ext[8] = {};
				if (!strcmp(texture->achFormatHint, "png"))
				{
					FILE* fp = nullptr;
					fopen_s(&fp, fullPath, "wb");
					if (!fp)
					{
						return nullptr;
					}

					fwrite(texture->pcData, texture->mWidth, 1, fp);

					fclose(fp);
				}
			}
			else
			{
				printf("doesn`t embedded texture.\n");
				return nullptr;
			}
		}
		else
		{
			return ret;
		}
	}
	else
	{
		return nullptr;
	}

	return ret;
}

MeshData ModelImporter::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	MeshData newMesh;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		vertex.normal.Normalize();

		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		newMesh.vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			newMesh.indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		const char* filePath = ReadTextureFromFilename(scene, material, aiTextureType_BASE_COLOR);
		if (!filePath)
		{
			filePath = ReadTextureFromFilename(scene, material, aiTextureType_DIFFUSE);
		}
		if (filePath)
		{
			strcpy_s(newMesh.albedoTextureFilaname, filePath);
		}
	}

	return newMesh;
}

void ModelImporter::PrintDebug()
{
	auto meshes = GetMeshes();

	int vertexCount = 0;
	int indexCount = 0;

	for (auto m : meshes)
	{
		for (auto v : m.vertices)
		{
			cout << v.position.x << " " << v.position.y << " " << v.position.z << endl;
			vertexCount++;
		}
	}

	for (auto m : meshes)
	{
		for (auto i : m.indices)
		{
			cout << i << " ";
			indexCount++;
		}
	}

	cout << endl;
	cout << "Vertex count: " << vertexCount << endl;
	cout << "Index count: " << indexCount << endl;
}
#endif