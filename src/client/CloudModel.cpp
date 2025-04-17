#include "pch.h"
#include "CloudModel.h"

CloudModel::CloudModel()
{
	if (!Initialize())
	{
		__debugbreak();
	}
}

CloudModel::~CloudModel()
{
	CleanUp();
}

AkBool CloudModel::Initialize()
{
	// Create Cube
	Vector3 vMin = Vector3(-1.0f);
	Vector3 vMax = Vector3(1.0f);
	MeshData_t* pCube = GeometryGenerator::MakeCube(&vMin, &vMax);

	_pCloudObj = GRenderer->CreateCloudObject();
	_pCloudObj->CreateMeshBuffers(pCube, 1);

	GeometryGenerator::DestroyGeometry(pCube, 1);

	_vMin = vMin;
	_vMax = vMax;

	// Set Name
	wcscpy_s(Name, L"Cloud");

	return AK_TRUE;
}

void CloudModel::Render()
{
	GRenderer->RenderCloud(_pCloudObj, fAnimSpeed, &_mWorldRow, fLightAbsorptionCoeff, &vLightDir, fDensityAbsorption, &vLightColor, fAniso);
}

void CloudModel::RenderGUI()
{
	// 해당 함수가 한번만 호출될 수 있도록 수정필요!
	// TODO:
	std::wstring ModelName = Name + std::wstring(L"_" + std::to_wstring(_uRefCount));
	char Title[_MAX_PATH] = {};
	strcpy_s(Title, ToString(ModelName + L" edit").c_str());

	ImGui::Begin(Title);
	ImGui::SliderFloat("Anim Speed", &fAnimSpeed, 0.0f, 0.001f, "%.6f");
	ImGui::SliderFloat("Light Absorption", &fLightAbsorptionCoeff, 0.0f, 10.0f);
	ImGui::SliderFloat3("Light Dir", &vLightDir.x, 0.0f, 1.0f);
	ImGui::SliderFloat("Density Absorption", &fDensityAbsorption, 0.0f, 50.0f);
	ImGui::SliderFloat3("Light Color", &vLightColor.x, 0.0f, 50.0f);
	ImGui::SliderFloat("Aniso", &fAniso, 0.0f, 1.0f);
	ImGui::End();
}

void CloudModel::GetMinMax(Vector3* pOutMin, Vector3* pOutMax)
{
	*pOutMin = _vMin;
	*pOutMax = _vMax;
}

void CloudModel::CleanUp()
{
	if (_pCloudObj)
	{
		_pCloudObj->Release();
		_pCloudObj = nullptr;
	}
}
