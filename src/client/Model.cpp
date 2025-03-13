#include "pch.h"
#include "Model.h"
#include "AssetManager.h"

/*
========
Model
========
*/

Model::Model(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	if (!Initialize(pMeshDataContainer, pAlbedo, fMetallic, fRoughness, pEmissive))
	{
		__debugbreak();
	}
}

Model::Model(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	if (!Initialize(pMeshData, uMeshDataNum, pAlbedo, fMetallic, fRoughness, pEmissive))
	{
		__debugbreak();
	}
}

Model::~Model()
{
	CleanUp();
}

AkBool Model::Initialize(AssetMeshDataContainer_t* pMeshDataContainer, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	CreateMeshObject(pMeshDataContainer->pMeshData, pMeshDataContainer->uMeshDataNum);
	CreateMaterial(pAlbedo, fMetallic, fRoughness, pEmissive);
	return AK_TRUE;
}

AkBool Model::Initialize(MeshData_t* pMeshData, AkU32 uMeshDataNum, const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	CreateMeshObject(pMeshData, uMeshDataNum);
	CreateMaterial(pAlbedo, fMetallic, fRoughness, pEmissive);
	return AK_TRUE;
}

void Model::Render()
{
	GRenderer->RenderBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::RenderNormal()
{
	GRenderer->RenderNormalOfBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::RenderShadow()
{
	GRenderer->RenderShadowOfBasicMeshObject(_pMeshObj, &_mWorldRow);
}

void Model::UpdateWorldRow(Matrix* pWorldRow)
{
	_mWorldRow = *pWorldRow;
}

void Model::SetWireFrame(AkBool bDrawWire)
{
	if (bDrawWire)
		_pMeshObj->EnableWireFrame();
	else
		_pMeshObj->DisableWireFrame();
}

void Model::CleanUp()
{
	if (_pMeshObj)
	{
		_pMeshObj->Release();
		_pMeshObj = nullptr;
	}
}

void Model::CreateMeshObject(MeshData_t* pMeshData, AkU32 uMeshDataNum)
{
	_pMeshObj = GRenderer->CreateBasicMeshObject();
	_pMeshObj->CreateMeshBuffers(pMeshData, uMeshDataNum);
}

void Model::CreateMaterial(const Vector3* pAlbedo, AkF32 fMetallic, AkF32 fRoughness, const Vector3* pEmissive)
{
	_pMeshObj->UpdateMaterialBuffers(pAlbedo, fMetallic, fRoughness, pEmissive);
}

/*
===========
Model Edit
===========
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
	_pScene = _pImporter->ReadFile(strFilaname.c_str(), aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_MaxQuality);
	if (!_pScene)
	{
		__debugbreak();
	}
}

void ModelExporter::ExportMaterial(const std::wstring& wcFilename)
{
#ifdef _DEBUG
	wprintf_s(L"\n[Load Materials]\n");
#endif
	LoadMaterial();
	std::wstring wcPath = L"../../assets/model_new/material/" + wcFilename + L".mat";
	SaveMaterial(wcPath);
}

void ModelExporter::ExportMesh(const std::wstring& wcFilename)
{
#ifdef _DEBUG
	wprintf_s(L"\n[Load Meshes]\n");
#endif
	LoadMeshes(_pScene->mRootNode);
	LoadNodes(_pScene->mRootNode, -1, -1);
	std::wstring wcPath = L"../../assets/model_new/mesh/" + wcFilename + L".mesh";
	SaveMesh(wcPath);
}

void ModelExporter::CleanUp()
{
	if (_pImporter)
	{
		delete _pImporter;
	}
}

void ModelExporter::LoadMaterial()
{
	for (AkU32 i = 0; i < _pScene->mNumMaterials; i++)
	{
		aiMaterial* pSrcMaterial = _pScene->mMaterials[i];
		MaterialData_t* pMaterial = new MaterialData_t;

		wstring wcTempName = ToWString(pSrcMaterial->GetName().C_Str());
		wcscpy_s(pMaterial->wcName, wcTempName.c_str());
#ifdef _DEBUG
		wprintf_s(L"Material Name: %s\n", pMaterial->wcName);
#endif

		aiColor3D tColor = {};
		pSrcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, tColor);
		pMaterial->vAmbient = Vector4(tColor.r, tColor.g, tColor.b, 1.0f);

		pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, tColor);
		pMaterial->vDiffuse = Vector4(tColor.r, tColor.g, tColor.b, 1.0f);

		pSrcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, tColor);
		pMaterial->vSpecular = Vector4(tColor.r, tColor.g, tColor.b, 1.0f);

		pSrcMaterial->Get(AI_MATKEY_SHININESS, pMaterial->vSpecular.w);

		pSrcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, tColor);
		pMaterial->vEmissive = Vector4(tColor.r, tColor.g, tColor.b, 1.0f);

		wcTempName = GetTextureName(pSrcMaterial, aiTextureType_BASE_COLOR);
		wcscpy_s(pMaterial->wcAlbedoMapName, wcTempName.c_str());
		if (!wcslen(pMaterial->wcAlbedoMapName))
		{
			wcTempName = GetTextureName(pSrcMaterial, aiTextureType_DIFFUSE);
			wcscpy_s(pMaterial->wcAlbedoMapName, wcTempName.c_str());
		}

		wcTempName = GetTextureName(pSrcMaterial, aiTextureType_EMISSIVE);
		wcscpy_s(pMaterial->wcEmissiveMapName, wcTempName.c_str());

		wcTempName = GetTextureName(pSrcMaterial, aiTextureType_HEIGHT);
		wcscpy_s(pMaterial->wcHeightMapName, wcTempName.c_str());

		wcTempName = GetTextureName(pSrcMaterial, aiTextureType_NORMALS);
		wcscpy_s(pMaterial->wcNormalMapName, wcTempName.c_str());

		wcTempName = GetTextureName(pSrcMaterial, aiTextureType_METALNESS);
		wcscpy_s(pMaterial->wcMetallicMapName, wcTempName.c_str());

		wcTempName = GetTextureName(pSrcMaterial, aiTextureType_DIFFUSE_ROUGHNESS);
		wcscpy_s(pMaterial->wcRoughnessMapName, wcTempName.c_str());

		wcTempName = GetTextureName(pSrcMaterial, aiTextureType_AMBIENT_OCCLUSION);
		wcscpy_s(pMaterial->wcAoMapName, wcTempName.c_str());
		if (!wcslen(pMaterial->wcAoMapName))
		{
			wcTempName = GetTextureName(pSrcMaterial, aiTextureType_LIGHTMAP);
			wcscpy_s(pMaterial->wcAoMapName, wcTempName.c_str());
		}

		wcTempName = GetTextureName(pSrcMaterial, aiTextureType_OPACITY);
		wcscpy_s(pMaterial->wcOpacityMapName, wcTempName.c_str());

		_vecMaterials.push_back(pMaterial);
	}
}

void ModelExporter::LoadNodes(aiNode* pNode, AkI32 iIndex, AkI32 iParent)
{
	NodeData_t* pNodeData = new NodeData_t;
	pNodeData->iIndex = iIndex;
	pNodeData->iParent = iParent;
	wcscpy_s(pNodeData->wcName, ToWString(pNode->mName.C_Str()).c_str());
#ifdef _DEBUG
	wprintf_s(L"%s\n", pNodeData->wcName);
#endif
	Matrix mMatrix(pNode->mTransformation[0]);
	mMatrix = mMatrix.Transpose();
	pNodeData->mTransform = mMatrix;

	_vecNodeData.push_back(pNodeData);

	for (AkU32 i = 0; i < pNode->mNumChildren; i++)
	{
		LoadNodes(pNode->mChildren[i], (AkI32)_vecNodeData.size(), iIndex);
	}
}

void ModelExporter::LoadMeshes(aiNode* pNode)
{
	for (AkU32 i = 0; i < pNode->mNumMeshes; i++)
	{
		New_MeshData_t* pMeshData = new New_MeshData_t;
		wcscpy_s(pMeshData->wcMeshName, ToWString(pNode->mName.C_Str()).c_str());

		AkU32 uIndex = pNode->mMeshes[i];
		aiMesh* pSrcMesh = _pScene->mMeshes[uIndex];

		aiMaterial* pMaterial = _pScene->mMaterials[pSrcMesh->mMaterialIndex];
		wcscpy_s(pMeshData->wcMaterialName, ToWString(pMaterial->GetName().C_Str()).c_str());

#ifdef _DEBUG
		wprintf_s(L"Mesh %u %s %s\n", i, pMeshData->wcMeshName, pMeshData->wcMaterialName);
#endif
		std::vector<VertexWeight_t> vecVertexWeights;
		vecVertexWeights.resize(pSrcMesh->mNumVertices);

		// Load Bone Data
		LoadBones(pSrcMesh, vecVertexWeights);

		pMeshData->uVerticeNum = pSrcMesh->mNumVertices;
#ifdef _DEBUG
		wprintf_s(L"Num Vertices: %u\n", pMeshData->uVerticeNum);
#endif
		if (!vecVertexWeights.empty())
		{
			pMeshData->pSkinnedVertices = new SkinnedVertex_t[pSrcMesh->mNumVertices];
		}
		else
		{
			pMeshData->pVertices = new Vertex_t[pSrcMesh->mNumVertices];
		}

		for (AkU32 j = 0; j < pSrcMesh->mNumVertices; j++)
		{
			if (!vecVertexWeights.empty())
			{
				SkinnedVertex_t tVert = {};
				memcpy(&tVert.vPosition, &pSrcMesh->mVertices[j], sizeof(Vector3));

				if (pSrcMesh->HasTextureCoords(0))
				{
					memcpy(&tVert.vTexCoord, &pSrcMesh->mTextureCoords[0][j], sizeof(Vector2));
				}
				if (pSrcMesh->HasNormals())
				{
					memcpy(&tVert.vNormalModel, &pSrcMesh->mNormals[j], sizeof(Vector3));
				}
				if (pSrcMesh->HasTangentsAndBitangents())
				{
					memcpy(&tVert.vTangentModel, &pSrcMesh->mTangents[j], sizeof(Vector3));
				}

				for (AkU32 k = 0; k < 8; k++)
				{
					tVert.uBoneIndices[k] = vecVertexWeights[j].uBoneIndices[k];
					tVert.fBlendWeights[k] = vecVertexWeights[j].fBlendWeights[k];
				}

				pMeshData->pSkinnedVertices[j] = tVert;
			}
			else
			{
				Vertex_t tVert = {};
				memcpy(&tVert.vPosition, &pSrcMesh->mVertices[j], sizeof(Vector3));

				if (pSrcMesh->HasTextureCoords(0))
				{
					memcpy(&tVert.vTexCoord, &pSrcMesh->mTextureCoords[0][j], sizeof(Vector2));
				}
				if (pSrcMesh->HasNormals())
				{
					memcpy(&tVert.vNormalModel, &pSrcMesh->mNormals[j], sizeof(Vector3));
				}
				if (pSrcMesh->HasTangentsAndBitangents())
				{
					memcpy(&tVert.vTangentModel, &pSrcMesh->mTangents[j], sizeof(Vector3));
				}

				pMeshData->pVertices[j] = tVert;
			}
		}

		AkU32 uIndiceNum = 0;
		for (AkU32 j = 0; j < pSrcMesh->mNumFaces; j++)
		{
			aiFace& tFace = pSrcMesh->mFaces[j];
			for (AkU32 k = 0; k < tFace.mNumIndices; k++)
			{
				uIndiceNum++;
			}
		}

		pMeshData->pIndices = new AkU32[uIndiceNum];
		pMeshData->uIndicesNum = uIndiceNum;
#ifdef _DEBUG
		wprintf_s(L"Num Indices: %u\n", pMeshData->uIndicesNum);
#endif
		for (AkU32 j = 0; j < pSrcMesh->mNumFaces; j++)
		{
			aiFace& tFace = pSrcMesh->mFaces[j];
			for (AkU32 k = 0; k < tFace.mNumIndices; k++)
			{
				pMeshData->pIndices[k] = tFace.mIndices[k];

				// TODO.
			}
		}

		_vecMeshes.push_back(pMeshData);
	}

	for (AkU32 i = 0; i < pNode->mNumChildren; i++)
	{
		LoadMeshes(pNode->mChildren[i]);
	}
}

void ModelExporter::LoadBones(aiMesh* pMesh, std::vector<VertexWeight_t>& vecVertexWeight)
{
	for (AkU32 i = 0; i < pMesh->mNumBones; i++)
	{
		AkU32 uBoneIndex = 0;
		wstring wcName = ToWString(pMesh->mBones[i]->mName.C_Str());
		if (!_mapBone.count(wcName))
		{
			uBoneIndex = _iBoneCount++;

			_mapBone[wcName] = uBoneIndex;

			BoneData_t* pBoneData = new BoneData_t;
			wcscpy_s(pBoneData->wcName, wcName.c_str());
			pBoneData->iIndex = uBoneIndex;

			Matrix mMatrix(pMesh->mBones[i]->mOffsetMatrix[0]);
			mMatrix = mMatrix.Transpose();
			pBoneData->mOffset = mMatrix;

			_vecBoneData.push_back(pBoneData);
		}
		else
		{
			uBoneIndex = _mapBone[wcName];
		}

		// TODO
		for (AkU32 j = 0; j < pMesh->mBones[i]->mNumWeights; j++)
		{
			AkU32 uIndex = pMesh->mBones[i]->mWeights[j].mVertexId;
			vecVertexWeight[uIndex].fBlendWeights[vecVertexWeight[uIndex].uWeightNum] = pMesh->mBones[i]->mWeights[j].mWeight;
			vecVertexWeight[uIndex].uBoneIndices[vecVertexWeight[uIndex].uIndiceNum] = uBoneIndex;
			vecVertexWeight[uIndex].uWeightNum++;
			vecVertexWeight[uIndex].uIndiceNum++;
		}
	}

#ifdef _DEBUG
	AkU8 uMaxWeightNum = 0;
	for (const auto& e : vecVertexWeight)
	{
		uMaxWeightNum = max(uMaxWeightNum, e.uWeightNum);
	}
	wprintf_s(L"Max weight num: %u\n", (AkU32)uMaxWeightNum);
#endif
}

void ModelExporter::SaveMaterial(const std::wstring& wcFilename)
{
	CreateFolders(ToString(wcFilename));

	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFilename.c_str(), L"wt");
	if (!fp)
	{
		__debugbreak();
	}

	std::wstring wcPath = GetFilePath(wcFilename);

	for (MaterialData_t* pMaterial : _vecMaterials)
	{
		fwprintf_s(fp, L"*** %s ***\n", pMaterial->wcName);
		fwprintf_s(fp, L"[Name]\n");
		fwprintf_s(fp, L"%s\n", CreateTexture(wcPath, pMaterial->wcAlbedoMapName).c_str());
		fwprintf_s(fp, L"%s\n", CreateTexture(wcPath, pMaterial->wcEmissiveMapName).c_str());
		fwprintf_s(fp, L"%s\n", CreateTexture(wcPath, pMaterial->wcHeightMapName).c_str());
		fwprintf_s(fp, L"%s\n", CreateTexture(wcPath, pMaterial->wcNormalMapName).c_str());
		fwprintf_s(fp, L"%s\n", CreateTexture(wcPath, pMaterial->wcMetallicMapName).c_str());
		fwprintf_s(fp, L"%s\n", CreateTexture(wcPath, pMaterial->wcRoughnessMapName).c_str());
		fwprintf_s(fp, L"%s\n", CreateTexture(wcPath, pMaterial->wcAoMapName).c_str());
		fwprintf_s(fp, L"%s\n", CreateTexture(wcPath, pMaterial->wcOpacityMapName).c_str());

		fwprintf_s(fp, L"[Ambient]\n");
		fwprintf_s(fp, L"%lf %lf %lf %lf\n", pMaterial->vAmbient.x, pMaterial->vAmbient.y, pMaterial->vAmbient.z, pMaterial->vAmbient.w);

		fwprintf_s(fp, L"[Ambient]\n");
		fwprintf_s(fp, L"%lf %lf %lf %lf\n", pMaterial->vDiffuse.x, pMaterial->vDiffuse.y, pMaterial->vDiffuse.z, pMaterial->vDiffuse.w);

		fwprintf_s(fp, L"[Ambient]\n");
		fwprintf_s(fp, L"%lf %lf %lf %lf\n", pMaterial->vSpecular.x, pMaterial->vSpecular.y, pMaterial->vSpecular.z, pMaterial->vSpecular.w);

		fwprintf_s(fp, L"[Ambient]\n");
		fwprintf_s(fp, L"%lf %lf %lf %lf\n", pMaterial->vEmissive.x, pMaterial->vEmissive.y, pMaterial->vEmissive.z, pMaterial->vEmissive.w);

		delete pMaterial;
	}

	_vecMaterials.clear();

	if (fp)
	{
		fclose(fp);
	}
}

void ModelExporter::SaveMesh(const std::wstring& wcFilename)
{
	CreateFolders(ToString(wcFilename));

	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFilename.c_str(), L"wt");
	if (!fp)
	{
		__debugbreak();
	}

	fwprintf_s(fp, L"%u\n", (AkU32)_vecMeshes.size());
	for (New_MeshData_t* pMeshData : _vecMeshes)
	{
		fwprintf_s(fp, L"%s\n", pMeshData->wcMeshName);
		fwprintf_s(fp, L"%s\n", pMeshData->wcMaterialName);
		fwprintf_s(fp, L"%u\n", pMeshData->uVerticeNum);
		if(pMeshData->pVertices)
			fwrite(pMeshData->pVertices, sizeof(Vertex_t) * pMeshData->uVerticeNum, 1, fp);
		if (pMeshData->pSkinnedVertices)
			fwrite(pMeshData->pSkinnedVertices, sizeof(SkinnedVertex_t) * pMeshData->uVerticeNum, 1, fp);
		fwprintf_s(fp, L"%u\n", pMeshData->uIndicesNum);
		fwrite(pMeshData->pIndices, sizeof(AkU32) * pMeshData->uIndicesNum, 1, fp);

		delete pMeshData;
	}
	_vecMeshes.clear();

	fwprintf_s(fp, L"%u\n", (AkU32)_vecNodeData.size());
	for (NodeData_t* pNodeData : _vecNodeData)
	{
		fwprintf_s(fp, L"%d\n", pNodeData->iIndex);
		fwprintf_s(fp, L"%s\n", pNodeData->wcName);
		fwprintf_s(fp, L"%d\n", pNodeData->iParent);
		fwrite(&pNodeData->mTransform, sizeof(Matrix), 1, fp);

		delete pNodeData;
	}
	_vecNodeData.clear();

	fwprintf_s(fp, L"%u\n", (AkU32)_vecNodeData.size());
	for (BoneData_t* pBoneData : _vecBoneData)
	{
		fwprintf_s(fp, L"%s\n", pBoneData->wcName);
		fwprintf_s(fp, L"%d\n", pBoneData->iIndex);
		fwrite(&pBoneData->mOffset, sizeof(Matrix), 1, fp);

		delete pBoneData;
	}
	_vecBoneData.clear();

	if (fp)
	{
		fclose(fp);
	}
}

void ModelExporter::SaveClip(const std::wstring& wcFilename)
{
}

std::wstring ModelExporter::GetTextureName(aiMaterial* pMaterial, aiTextureType eTyep)
{
	aiString strFile = {};
	std::wstring wcRet = L"";
	pMaterial->GetTexture(eTyep, 0, &strFile);
	wcRet = ToWString(strFile.C_Str());
#ifdef _DEBUG
	wprintf_s(L"File name: %s\n", wcRet.c_str());
#endif
	return wcRet;
}

std::wstring ModelExporter::CreateTexture(const std::wstring& wcPath, const std::wstring& wcFileName)
{
	using namespace DirectX;

	if (!wcFileName.length())
		return L"";

	std::wstring wcFile = GetFileNmaeExcludeExt(GetFileName(wcFileName)) + L".png";
	std::string strTemp = ToString(wcFile);
	const aiTexture* pTexture = _pScene->GetEmbeddedTexture(strTemp.c_str());

	std::wstring wcFullPath = wcPath + wcFile;
	if (GetFileAttributes(wcFullPath.c_str()) < 0xffffffff)
	{
		return wcFile;
	}

	if (pTexture)
	{
		if (pTexture->mHeight < 1)
		{
			FILE* fp = nullptr;
			_wfopen_s(&fp, wcFullPath.c_str(), L"wb");
			if (!fp) __debugbreak();

			fwrite(pTexture->pcData, pTexture->mWidth, 1, fp);

			if (fp) fclose(fp);
		}
		else
		{
			DirectX::Image tImage = {};
			tImage.width = pTexture->mWidth;
			tImage.height = pTexture->mHeight;
			tImage.pixels = (uint8_t*)(pTexture->pcData);
			tImage.format = DXGI_FORMAT_R8G8B8A8_UNORM;
			tImage.rowPitch = tImage.width * 4;
			tImage.slicePitch = tImage.width * tImage.height * 4;

			DirectX::SaveToWICFile(tImage, WIC_FLAGS_NONE, GetWICCodec(WIC_CODEC_PNG), wcFullPath.c_str());
		}
	}

	return wcFile;
}

ModelEdit::ModelEdit()
{
}

ModelEdit::~ModelEdit()
{
}

AkBool ModelEdit::Initialize()
{
	return AkBool();
}

void ModelEdit::Render()
{
}

void ModelEdit::Load()
{
}

void ModelEdit::Save()
{
}

void ModelEdit::CleanUp()
{
}
