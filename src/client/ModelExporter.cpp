#include "pch.h"
#include "ModelExporter.h"

/*
===============
Model Exporter
===============
*/

using std::string;
using std::wstring;

ModelExporter::ModelExporter(const std::string& strFilaname)
{
	if (!Initialize())
	{
		__debugbreak();
	}

	Load(strFilaname);
}

ModelExporter::~ModelExporter()
{
	CleanUp();
}

AkBool ModelExporter::Initialize()
{
	_pImporter = new Assimp::Importer();
	if (!_pImporter)
	{
		return AK_FALSE;
	}
	return AK_TRUE;
}

void ModelExporter::Load(const std::string& strFilaname)
{
	if (GetFileExtension(strFilaname) == ".gltf")
	{
		_bIsGLTF = AK_TRUE;
		_bRevertNormal = AK_TRUE;
	}

	_pScene = _pImporter->ReadFile(strFilaname.c_str(), aiProcess_ConvertToLeftHanded | aiProcess_Triangulate);
	if (!_pScene)
	{
		std::cout << "Failed to read file: " << strFilaname << std::endl;
		auto errorDescription = _pImporter->GetErrorString();
		std::cout << "Assimp error: " << errorDescription << std::endl;
		__debugbreak();
	}

	_wcFilename = ToWString(GetFileName(GetFileNmaeExcludeExt(strFilaname)));
	_wcFolder = ToWString(GetCurrentFolder(strFilaname));

	// 1. 모든 메쉬에 대해서 버텍스에 영향을 주는 뼈들의 목록을 만든다.
	FindDeformingBones(_pScene);

	// 2. 트리 구조를 따라 업데이트 순서대로 뼈들의 인덱스를 결정한다
	int counter = 0;
	UpdateBoneIDs(_pScene->mRootNode, &counter);

	// 3. 업데이트 순서대로 뼈 이름 저장 (boneIdToName)
	_tAnimData.boneIdToName.resize(_tAnimData.boneNameToId.size());
	for (auto& i : _tAnimData.boneNameToId)
	{
		_tAnimData.boneIdToName[i.second] = i.first;
	}

	// 각 뼈의 부모 인덱스를 저장할 준비
	_tAnimData.boneParents.resize(_tAnimData.boneNameToId.size(), -1);

	Matrix tr; // Initial transformation
	ProcessNode(_pScene->mRootNode, _pScene, tr);

	// 애니메이션 정보 읽기
	if (_pScene->HasAnimations())
	{
		ReadAnimation(_pScene);
	}

	UpdateTangents();
}

const aiNode* ModelExporter::FindParent(const aiNode* node)
{
	if (!node)
		return nullptr;
	if (_tAnimData.boneNameToId.count(node->mName.C_Str()) > 0)
		return node;
	return FindParent(node->mParent);
}

void ModelExporter::ProcessNode(aiNode* node, const aiScene* scene, DirectX::SimpleMath::Matrix tr)
{
	if (node->mParent && _tAnimData.boneNameToId.count(node->mName.C_Str()) && FindParent(node->mParent))
	{
		const auto boneId = _tAnimData.boneNameToId[node->mName.C_Str()];
		_tAnimData.boneParents[boneId] = _tAnimData.boneNameToId[FindParent(node->mParent)->mName.C_Str()];
	}

	Matrix m(&node->mTransformation.a1);
	m = m.Transpose() * tr;

	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		auto newMesh = this->ProcessMesh(mesh, scene);
		for (auto& v : newMesh.vertices)
		{
			v.vPosition = DirectX::SimpleMath::Vector3::Transform(v.vPosition, m);
		}
		_vecModelMeshData.push_back(newMesh);
	}

	for (UINT i = 0; i < node->mNumChildren; i++) {
		this->ProcessNode(node->mChildren[i], scene, m);
	}
}

ModelMeshData ModelExporter::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	ModelMeshData newMesh;
	auto& vertices = newMesh.vertices;
	auto& indices = newMesh.indices;
	auto& skinnedVertices = newMesh.skinnedVertices;

	// Walk through each of the mesh's vertices
	for (AkU32 i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex_t vertex;

		vertex.vPosition.x = mesh->mVertices[i].x;
		vertex.vPosition.y = mesh->mVertices[i].y;
		vertex.vPosition.z = mesh->mVertices[i].z;

		vertex.vNormalModel.x = mesh->mNormals[i].x;
		if (_bIsGLTF)
		{
			vertex.vNormalModel.y = mesh->mNormals[i].z;
			vertex.vNormalModel.z = -mesh->mNormals[i].y;
		}
		else
		{
			vertex.vNormalModel.y = mesh->mNormals[i].y;
			vertex.vNormalModel.z = mesh->mNormals[i].z;
		}

		if (_bRevertNormal)
		{
			vertex.vNormalModel *= -1.0f;
		}

		vertex.vNormalModel.Normalize();

		if (mesh->mTextureCoords[0]) {
			vertex.vTexCoord.x = (AkF32)mesh->mTextureCoords[0][i].x;
			vertex.vTexCoord.y = (AkF32)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (AkU32 i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (AkU32 j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->HasBones()) {

		vector<vector<float>> boneWeights(vertices.size());
		vector<vector<uint8_t>> boneIndices(vertices.size());

		_tAnimData.offsetMatrices.resize(_tAnimData.boneNameToId.size());
		_tAnimData.boneTransforms.resize(_tAnimData.boneNameToId.size());

		AkI32 iCount = 0;
		for (uint32_t i = 0; i < mesh->mNumBones; i++)
		{
			const aiBone* bone = mesh->mBones[i];

			const uint32_t boneId = _tAnimData.boneNameToId[bone->mName.C_Str()];

			_tAnimData.offsetMatrices[boneId] = Matrix((float*)&bone->mOffsetMatrix).Transpose();

			// 이 뼈가 영향을 주는 Vertex의 개수
			for (uint32_t j = 0; j < bone->mNumWeights; j++)
			{
				aiVertexWeight weight = bone->mWeights[j];
				assert(weight.mVertexId < boneIndices.size());
				boneIndices[weight.mVertexId].push_back(boneId);
				boneWeights[weight.mVertexId].push_back(weight.mWeight);
			}
		}

		int maxBones = 0;
		for (int i = 0; i < boneWeights.size(); i++) {
			maxBones = std::max(maxBones, int(boneWeights[i].size()));
		}

		wprintf_s(L"Max number of influencing bones per vertex = %d\n", maxBones);

		skinnedVertices.resize(vertices.size());
		for (int i = 0; i < vertices.size(); i++)
		{
			skinnedVertices[i].vPosition = vertices[i].vPosition;
			skinnedVertices[i].vNormalModel = vertices[i].vNormalModel;
			skinnedVertices[i].vTexCoord = vertices[i].vTexCoord;

			for (int j = 0; j < boneWeights[i].size(); j++)
			{
				skinnedVertices[i].fBlendWeights[j] = boneWeights[i][j];
				skinnedVertices[i].uBoneIndices[j] = boneIndices[i][j];
			}
		}
	}

	// http://assimp.sourceforge.net/lib_html/materials.html
	if (mesh->mMaterialIndex >= 0) {

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		newMesh.albedoTextureFilename = ReadTextureFilename(scene, material, aiTextureType_BASE_COLOR);
		if (newMesh.albedoTextureFilename.empty())
		{
			newMesh.albedoTextureFilename = ReadTextureFilename(scene, material, aiTextureType_DIFFUSE);
		}
		newMesh.emissiveTextureFilename = ReadTextureFilename(scene, material, aiTextureType_EMISSIVE);
		newMesh.heightTextureFilename = ReadTextureFilename(scene, material, aiTextureType_HEIGHT);
		newMesh.normalTextureFilename = ReadTextureFilename(scene, material, aiTextureType_NORMALS);
		newMesh.metallicTextureFilename = ReadTextureFilename(scene, material, aiTextureType_METALNESS);
		newMesh.roughnessTextureFilename = ReadTextureFilename(scene, material, aiTextureType_DIFFUSE_ROUGHNESS);
		newMesh.aoTextureFilename = ReadTextureFilename(scene, material, aiTextureType_AMBIENT_OCCLUSION);
		if (newMesh.aoTextureFilename.empty())
		{
			newMesh.aoTextureFilename = ReadTextureFilename(scene, material, aiTextureType_LIGHTMAP);
		}
		newMesh.opacityTextureFilename = ReadTextureFilename(scene, material, aiTextureType_OPACITY);

		if (!newMesh.opacityTextureFilename.empty())
		{
			printf_s("%s\n", newMesh.albedoTextureFilename.c_str());
			printf_s("Opacity: %s", newMesh.opacityTextureFilename.c_str());
		}
	}

	return newMesh;
}

void ModelExporter::ReadAnimation(const aiScene* scene)
{
	_tAnimData.clips.resize(scene->mNumAnimations);

	for (uint32_t i = 0; i < scene->mNumAnimations; i++) {

		auto& clip = _tAnimData.clips[i];

		const aiAnimation* ani = scene->mAnimations[i];

		clip.duration = ani->mDuration;
		clip.ticksPerSec = ani->mTicksPerSecond;
		clip.keys.resize(_tAnimData.boneNameToId.size());
		clip.numChannels = ani->mNumChannels;

		for (uint32_t c = 0; c < ani->mNumChannels; c++) {
			const aiNodeAnim* nodeAnim = ani->mChannels[c];
			const int boneId =
				_tAnimData.boneNameToId[nodeAnim->mNodeName.C_Str()];
			clip.keys[boneId].resize(nodeAnim->mNumPositionKeys);
			for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++) {
				const auto pos = nodeAnim->mPositionKeys[k].mValue;
				const auto rot = nodeAnim->mRotationKeys[k].mValue;
				const auto scale = nodeAnim->mScalingKeys[k].mValue;

				auto& key = clip.keys[boneId][k];
				key.pos = { pos.x, pos.y, pos.z };
				key.rot = Quaternion(rot.x, rot.y, rot.z, rot.w);
				key.scale = { scale.x, scale.y, scale.z };
			}
		}
	}
}

std::string ModelExporter::ReadTextureFilename(const aiScene* scene, aiMaterial* material, aiTextureType type)
{
	const std::string strTexturePath = "../../assets/model/textures/";

	CreateFolders(strTexturePath);

	if (material->GetTextureCount(type) > 0) {
		aiString filepath;
		material->GetTexture(type, 0, &filepath);

		string fullPath = strTexturePath + string(std::filesystem::path(filepath.C_Str()).filename().string());

		// 1. 실제로 파일이 존재하는지 확인
		if (!std::filesystem::exists(fullPath)) {
			// 2. 파일이 없을 경우 혹시 fbx 자체에 Embedded인지 확인
			const aiTexture* texture =
				scene->GetEmbeddedTexture(filepath.C_Str());
			if (texture) {
				// 3. Embedded texture가 존재하고 png일 경우 저장
				if (string(texture->achFormatHint).find("png") !=
					string::npos) {

					string texPath = strTexturePath + string(std::filesystem::path(filepath.C_Str()).filename().string());

					ofstream fs(texPath.c_str(), std::ios::binary | std::ios::out);
					fs.write((char*)texture->pcData, texture->mWidth);
					fs.close();
					// 참고: compressed format일 경우 texture->mHeight가 0

					// Convert PNG to DDS.
					SaveDDS(ToWString(texPath).c_str(), AK_FALSE);

					DeleteFile(ToWString(texPath).c_str());
				}
			}
			else {
				std::cout << fullPath << " doesn't exists. Return empty filename." << std::endl;
			}
		}
		else {

			DeleteFile(ToWString(fullPath).c_str());

			return fullPath;
		}

		return fullPath;

	}
	else {
		return "";
	}
}

void ModelExporter::UpdateTangents()
{
	using namespace std;
	using namespace DirectX;

	// https://github.com/microsoft/DirectXMesh/wiki/ComputeTangentFrame

	for (auto& m : this->_vecModelMeshData) {

		vector<XMFLOAT3> positions(m.vertices.size());
		vector<XMFLOAT3> normals(m.vertices.size());
		vector<XMFLOAT2> texcoords(m.vertices.size());
		vector<XMFLOAT3> tangents(m.vertices.size());
		vector<XMFLOAT3> bitangents(m.vertices.size());

		for (size_t i = 0; i < m.vertices.size(); i++) {
			auto& v = m.vertices[i];
			positions[i] = v.vPosition;
			normals[i] = v.vNormalModel;
			texcoords[i] = v.vTexCoord;
		}

		ComputeTangentFrame(m.indices.data(), m.indices.size() / 3, positions.data(), normals.data(), texcoords.data(), m.vertices.size(), tangents.data(), bitangents.data());

		for (size_t i = 0; i < m.vertices.size(); i++)
		{
			m.vertices[i].vTangentModel = tangents[i];
		}

		if (m.skinnedVertices.size() > 0)
		{
			for (size_t i = 0; i < m.skinnedVertices.size(); i++)
			{
				m.skinnedVertices[i].vTangentModel = tangents[i];
			}
		}
	}
}

void ModelExporter::FindDeformingBones(const aiScene* scene)
{
	for (uint32_t i = 0; i < scene->mNumMeshes; i++)
	{
		const auto* mesh = scene->mMeshes[i];
		if (mesh->HasBones()) {
			for (uint32_t i = 0; i < mesh->mNumBones; i++)
			{
				const aiBone* bone = mesh->mBones[i];

				_tAnimData.boneNameToId[bone->mName.C_Str()] = -1;
			}
		}
	}
}

void ModelExporter::UpdateBoneIDs(aiNode* node, int* counter)
{
	static int id = 0;
	if (node)
	{
		if (_tAnimData.boneNameToId.count(node->mName.C_Str()))
		{
			_tAnimData.boneNameToId[node->mName.C_Str()] = *counter;
			*counter += 1;
		}
		for (UINT i = 0; i < node->mNumChildren; i++)
		{
			UpdateBoneIDs(node->mChildren[i], counter);
		}
	}
}

void ModelExporter::ExportMesh()
{
#ifdef _DEBUG
	wprintf_s(L"\n[Load Meshes]\n");
#endif

	std::wstring wcPath = L"../../assets/model/mesh/" + _wcFilename + L".mesh";
	SaveMesh(wcPath);

#ifdef _DEBUG
	wprintf_s(L"\n[Load Meshes End]\n");
#endif
}

void ModelExporter::ExportClip()
{
#ifdef _DEBUG
	wprintf_s(L"\n[Load Clips]\n");
#endif

	std::wstring wcPath = L"../../assets/model/animation/" + _wcFolder + L"/" + _wcFilename + L".anim";
	SaveClip(wcPath);

#ifdef _DEBUG
	wprintf_s(L"\n[Load Clips End]\n");
#endif
}

void ModelExporter::CleanUp()
{
	if (_pImporter)
	{
		delete _pImporter;
	}
}

void ModelExporter::SaveMesh(const std::wstring& wcPath)
{
	using namespace std;

	CreateFolders(ToString(wcPath));

	std::vector<ModelMeshData> meshes;
	meshes = _vecModelMeshData;

	ofstream fout;
	fout.open(ToString(wcPath).c_str());

	// write mesh data.
	fout << "========MeshData========" << endl;
	fout << "MeshCount: " << meshes.size() << endl;
	for (int i = 0; i < meshes.size(); i++)
	{
		if (_tAnimData.boneNameToId.size())
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
		fout << "Albedo: " << (meshes[i].albedoTextureFilename.empty() ? "Empty" : GetFileNmaeExcludeExt(meshes[i].albedoTextureFilename) + ".dds") << endl;
		fout << "Emissive: " << (meshes[i].emissiveTextureFilename.empty() ? "Empty" : GetFileNmaeExcludeExt(meshes[i].emissiveTextureFilename) + ".dds") << endl;
		fout << "Height: " << (meshes[i].heightTextureFilename.empty() ? "Empty" : GetFileNmaeExcludeExt(meshes[i].heightTextureFilename) + ".dds") << endl;
		fout << "Normal: " << (meshes[i].normalTextureFilename.empty() ? "Empty" : GetFileNmaeExcludeExt(meshes[i].normalTextureFilename) + ".dds") << endl;
		fout << "Metallic: " << (meshes[i].metallicTextureFilename.empty() ? "Empty" : GetFileNmaeExcludeExt(meshes[i].metallicTextureFilename) + ".dds") << endl;
		fout << "Roughness: " << (meshes[i].roughnessTextureFilename.empty() ? "Empty" : GetFileNmaeExcludeExt(meshes[i].roughnessTextureFilename) + ".dds") << endl;
		fout << "Ao: " << (meshes[i].aoTextureFilename.empty() ? "Empty" : GetFileNmaeExcludeExt(meshes[i].aoTextureFilename) + ".dds") << endl;
		fout << "Opacity: " << (meshes[i].opacityTextureFilename.empty() ? "Empty" : GetFileNmaeExcludeExt(meshes[i].opacityTextureFilename) + ".dds") << endl;
		fout << endl;
	}

	// write vertices and indices.
	fout << "========Vertices========" << endl;
	for (int i = 0; i < meshes.size(); i++)
	{
		if (_tAnimData.boneNameToId.size())
		{
			for (int j = 0; j < meshes[i].skinnedVertices.size(); j++)
			{
				fout << "Position: " << meshes[i].skinnedVertices[j].vPosition.x << " " << meshes[i].skinnedVertices[j].vPosition.y << " " << meshes[i].skinnedVertices[j].vPosition.z << endl;
				fout << "Normal: " << meshes[i].skinnedVertices[j].vNormalModel.x << " " << meshes[i].skinnedVertices[j].vNormalModel.y << " " << meshes[i].skinnedVertices[j].vNormalModel.z << endl;
				fout << "Texcoord: " << meshes[i].skinnedVertices[j].vTexCoord.x << " " << meshes[i].skinnedVertices[j].vTexCoord.y << endl;
				fout << "Tangent: " << meshes[i].skinnedVertices[j].vTangentModel.x << " " << meshes[i].skinnedVertices[j].vTangentModel.y << " " << meshes[i].skinnedVertices[j].vTangentModel.z << endl;
				fout << "BlendWeight: ";
				for (int k = 0; k < 8; k++)
				{
					fout << meshes[i].skinnedVertices[j].fBlendWeights[k] << " ";
				}
				fout << endl;
				fout << "BoneIndices: ";
				for (int k = 0; k < 8; k++)
				{
					fout << (int)meshes[i].skinnedVertices[j].uBoneIndices[k] << " ";
				}
				fout << endl;
				fout << endl;
			}
		}
		else
		{
			for (int j = 0; j < meshes[i].vertices.size(); j++)
			{
				fout << "Position: " << meshes[i].vertices[j].vPosition.x << " " << meshes[i].vertices[j].vPosition.y << " " << meshes[i].vertices[j].vPosition.z << endl;
				fout << "Normal: " << meshes[i].vertices[j].vNormalModel.x << " " << meshes[i].vertices[j].vNormalModel.y << " " << meshes[i].vertices[j].vNormalModel.z << endl;
				fout << "Texcoord: " << meshes[i].vertices[j].vTexCoord.x << " " << meshes[i].vertices[j].vTexCoord.y << endl;
				fout << "Tangent: " << meshes[i].vertices[j].vTangentModel.x << " " << meshes[i].vertices[j].vTangentModel.y << " " << meshes[i].vertices[j].vTangentModel.z << endl;
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

	AnimationData animData;
	animData = _tAnimData;

	// write bone offset matrix
	fout << "========BoneOffsets========" << endl;
	fout << "BoneCount: " << animData.offsetMatrices.size() << endl;
	for (int i = 0; i < animData.offsetMatrices.size(); i++)
	{
		fout << "BoneOffset#" << i << endl;
		fout << animData.offsetMatrices[i]._11 << " " << animData.offsetMatrices[i]._12 << " " << animData.offsetMatrices[i]._13 << " " << animData.offsetMatrices[i]._14 << endl;
		fout << animData.offsetMatrices[i]._21 << " " << animData.offsetMatrices[i]._22 << " " << animData.offsetMatrices[i]._23 << " " << animData.offsetMatrices[i]._24 << endl;
		fout << animData.offsetMatrices[i]._31 << " " << animData.offsetMatrices[i]._32 << " " << animData.offsetMatrices[i]._33 << " " << animData.offsetMatrices[i]._34 << endl;
		fout << animData.offsetMatrices[i]._41 << " " << animData.offsetMatrices[i]._42 << " " << animData.offsetMatrices[i]._43 << " " << animData.offsetMatrices[i]._44 << endl;
		fout << endl;
	}

	// write bone hierarchy
	fout << "========BoneHierarchy========" << endl;
	for (int i = 0; i < animData.boneParents.size(); i++)
	{
		fout << "ParentIndexOfBone" << i << ": " << animData.boneParents[i] << endl;
	}
	fout << endl;

	// write bone name
	fout << "========BoneName========" << endl;
	for (int i = 0; i < animData.boneIdToName.size(); i++)
	{
		fout << animData.boneIdToName[i] << endl;
	}
	fout << endl;

	if (fout.is_open())
	{
		fout.close();
	}
}

void ModelExporter::SaveClip(const std::wstring& wcPath)
{
	using namespace std;

#ifdef _DEBUG
	wprintf_s(L"\n[Save Clips Start]\n");
#endif

	CreateFolders(ToString(wcPath));

	ofstream fout;
	fout.open(ToString(wcPath).c_str());

	AnimationData animData = _tAnimData;

	// write animation clip
	fout << "========AnimationClip========" << endl;
	fout << "AnimationClipCount: " << 1 << endl;
	for (int i = 0; i < animData.clips.size(); i++)
	{
		fout << "AnimationClip" << i << ": " << ToString(GetFileName(wcPath)) << endl;
		fout << "Duration: " << animData.clips[i].duration << endl;
		fout << "TicksPerSecond: " << animData.clips[i].ticksPerSec << endl;
		fout << "{" << endl;
		for (int j = 0; j < animData.clips[i].keys.size(); j++)
		{
			fout << "\tBone" << j << " " << "KeyFrame: " << animData.clips[i].keys[j].size() << endl;
			fout << "\t{" << endl;
			for (int k = 0; k < animData.clips[i].keys[j].size(); k++)
			{
				fout << "\t\tTime: " << k << endl;
				fout << "\t\t\tPos: " << animData.clips[i].keys[j][k].pos.x << " " << animData.clips[i].keys[j][k].pos.y << " " << animData.clips[i].keys[j][k].pos.z << endl;
				fout << "\t\t\tScale: " << animData.clips[i].keys[j][k].scale.x << " " << animData.clips[i].keys[j][k].scale.y << " " << animData.clips[i].keys[j][k].scale.z << endl;
				fout << "\t\t\tQuat: " << animData.clips[i].keys[j][k].rot.x << " " << animData.clips[i].keys[j][k].rot.y << " " << animData.clips[i].keys[j][k].rot.z << " " << animData.clips[i].keys[j][k].rot.w << endl;
			}
			fout << "\t}" << endl;
		}
		fout << "}" << endl;
	}

	if (fout.is_open())
	{
		fout.close();
	}

#ifdef _DEBUG
	wprintf_s(L"\n[Save Clips End]\n");
#endif
}