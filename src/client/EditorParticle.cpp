#include "pch.h"
#include "EditorParticle.h"
#include "Camera.h"

/*
=====================
Particle Editor
=====================
*/

EditorParticle::EditorParticle()
{
    if (!Initialize())
    {
        __debugbreak();
    }
}

EditorParticle::~EditorParticle()
{
    CleanUp();
}

AkBool EditorParticle::Initialize()
{
    Vector3 vCamPos = Vector3(0.0f, 0.0f, -10.0f);
    Vector3 vCamYawPitchRoll = Vector3(0.0f, 0.0f, 0.0f);
    _pCamera = new Camera(&vCamPos, &vCamYawPitchRoll);
    _pCamera->Mode = CAMERA_MODE::EDITOR;

    return AK_TRUE;
}

AkBool EditorParticle::BeginEditor()
{
    return AK_TRUE;
}

AkBool EditorParticle::EndEditor()
{
    return AK_TRUE;
}

void EditorParticle::Update()
{
    if(_bFPV)
    {
        _pCamera->Update();
    }

    if (_pParticle)
    {
        _pParticle->Update();
    }
}

void EditorParticle::FinalUpdate()
{
    if (KEY_DOWN(KEY_INPUT_F))
    {
        _bFPV = !_bFPV;
    }

    _pCamera->UpdateEditor();

    if(_pParticle)
    {
        ImGui::Begin("[Transform]");
        Vector3 vPos = _pParticle->GetTransform()->GetPosition();
        ImGui::InputFloat3("Position", (AkF32*)&vPos);
        _pParticle->GetTransform()->SetPosition(&vPos);
        ImGui::End();
    }
    
    ImGui::Begin("[Particle Editor]");
    ImGui::Checkbox("FPV", &_bFPV);
    if (ImGui::Button("Create")) CreateParticle();
    if (ImGui::Button("Play")) Play();
    if (ImGui::Button("Stop")) Stop();
    if (ImGui::Button("Clear")) DestroyParticle();
    if (ImGui::Button("Save")) Save(L"spark");
    if (ImGui::Button("Load")) Load(L"spark");
    ImGui::End();

    ImGui::Begin("[Particle Info]");
   
    // Loop.
    ImGui::Text("Loop");
    ImGui::Checkbox("lp", &_tInfo.bLoop);
    
    // Shape.
    ImGui::Text("Shape");
    ImGui::RadioButton("sphere", &_iShape, 0); ImGui::SameLine();
    ImGui::RadioButton("circle", &_iShape, 1); ImGui::SameLine();
    ImGui::RadioButton("cone", &_iShape, 2);
    _tInfo.iShape = _iShape;
    
    // Textures.
    ImGui::Text("Textures");
    ImGui::ListBox("Images", &_iSelectedItem, _cItems, IM_ARRAYSIZE(_cItems));

    // Count.
    ImGui::Text("Count");
    ImGui::InputInt("ct", (AkI32*)&_tInfo.uCount);
    
    // Duration.
    ImGui::Text("Duration");
    ImGui::InputFloat("dr", &_tInfo.fDuration);

    // Start Life Time.
    ImGui::Text("Start Life Time");
    ImGui::InputFloat2("lt", (AkF32*)&_tInfo.vStartLifeTime);

    // Start Speed.
    ImGui::Text("Start Speed");
    ImGui::InputFloat2("sp", (AkF32*)&_tInfo.vStartSpeed);

    // Start Radius.
    ImGui::Text("Start Radius");
    ImGui::InputFloat2("rd", (AkF32*)&_tInfo.vStartRadius);

    // Start Size.
    ImGui::Text("Start Size");
    ImGui::InputFloat2("sz", (AkF32*)&_tInfo.vStartSize);

    // Start Color.
    ImGui::Text("Start Color");
    ImGui::ColorEdit4("cr1", (AkF32*)&_tInfo.vStartColor[0]);
    ImGui::ColorEdit4("cr2", (AkF32*)&_tInfo.vStartColor[1]);

    // Color Over life time.
    ImGui::Text("Color Over Life Time");
    ImGui::ColorEdit4("colt", (AkF32*)&_tInfo.vColorOverLifeTime);

    ImGui::End();
}

void EditorParticle::Render()
{
    if (_pParticle)
    {
        _pParticle->Render();
    }
}

void EditorParticle::RenderShadow()
{

}

void EditorParticle::Play()
{
    if (_pParticle)
    {
        Vector3 vPos = _pParticle->GetTransform()->GetPosition();

        _pParticle->Play(&vPos);
    }
}

void EditorParticle::Stop()
{
    if (_pParticle)
    {
        _pParticle->Stop();
    }
}

void EditorParticle::Load(const std::wstring& wcFilePath)
{
    DestroyParticle();

    std::wstring wcFullPath = L"../../assets/particle/" + wcFilePath + L".fx";

    FILE* fp = nullptr;
    _wfopen_s(&fp, wcFullPath.c_str(), L"rb");
    if (!fp) __debugbreak();

    Spark::ParticleInfo_t tInfo = {};
    fread(&tInfo, sizeof(Spark::ParticleInfo_t), 1, fp);
    fread(&_iSelectedItem, sizeof(AkI32), 1, fp);

    _tInfo = tInfo;
    _iShape = tInfo.iShape;

    CreateParticle();

    if (fp) fclose(fp);
}

void EditorParticle::Save(const std::wstring& wcFilePath)
{
    std::wstring wcFullPath = L"../../assets/particle/" + wcFilePath + L".fx";

    FILE* fp = nullptr;
    _wfopen_s(&fp, wcFullPath.c_str(), L"wb");
    if (!fp) __debugbreak();

    fwrite(&_tInfo, sizeof(Spark::ParticleInfo_t), 1, fp);
    fwrite(&_iSelectedItem,  sizeof(AkI32), 1, fp);

    if (fp) fclose(fp);
}

void EditorParticle::CleanUp()
{
    DestroyParticle();

    if(_pCamera)
    {
        delete _pCamera;
        _pCamera = nullptr;
    }
}

void EditorParticle::CreateParticle()
{
    DestroyParticle();

    std::wstring wcFilePath = L"../../assets/particle/" + ToWString(_cItems[_iSelectedItem]) + L".dds";
    _pParticle = new Spark(wcFilePath.c_str(), &_tInfo);
}

void EditorParticle::DestroyParticle()
{
    if (_pParticle)
    {
        delete _pParticle;
        _pParticle = nullptr;
    }
}
