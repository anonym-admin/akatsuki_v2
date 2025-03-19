#include "pch.h"
#include "EditorModel.h"
#include "ModelExporter.h"

EditorModel::EditorModel()
{

}

EditorModel::~EditorModel()
{
	EndEditor();
}

AkBool EditorModel::BeginEditor()
{
	return AK_TRUE;
}

AkBool EditorModel::EndEditor()
{
	return AK_TRUE;
}

void EditorModel::Update()
{

}

void EditorModel::FinalUpdate()
{
	ImGui::Begin("Export");

	IGFD::FileDialogConfig tConfig = {};
	tConfig.filePathName = "../../assets/model_new/origin/";

	const char* pItems[] = { "Model", "Animation" };
	if (ImGui::Combo("Texture Type", &_iExportType, pItems, IM_ARRAYSIZE(pItems)))
	{
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".fbx,.gltf,.obj", tConfig);
	}

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			std::string FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			switch (_iExportType)
			{
			case 0:
				CreateModel(ToWString(GetFileNmaeExcludeExt(GetFileName(FileName))), ToWString(GetFileExtension(FileName)));
				break;
			case 1:
				CreateAnimation(ToWString(GetFileName(FilePath)), ToWString(GetFileNmaeExcludeExt(GetFileName(FileName))));
				break;
			}
		}

		ImGuiFileDialog::Instance()->Close();
	}

	ImGui::End();
}

void EditorModel::Render()
{

}

void EditorModel::RenderShadow()
{

}

void EditorModel::Load(const std::wstring& wcFilePath)
{
}

void EditorModel::Save(const std::wstring& wcFilePath)
{
}

void EditorModel::CreateModel(const std::wstring& wcName, const std::wstring& wcExt)
{
	_pExporter = new ModelExporter(ToString(L"../../assets/model_new/origin/models/" + wcName + L"." + wcExt));
	_pExporter->ExportMesh();
	delete _pExporter;
}

void EditorModel::CreateAnimation(const std::wstring& wcName, const std::wstring& wcClip)
{
	_pExporter = new ModelExporter(ToString(L"../../assets/model_new/origin/animations/" + wcName + L"/" + wcClip));
	_pExporter->ExportClip();
	delete _pExporter;
}

