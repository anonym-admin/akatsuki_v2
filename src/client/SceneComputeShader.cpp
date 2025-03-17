#include "pch.h"
#include "SceneComputeShader.h"
#include <random>

SceneComputeShader::~SceneComputeShader()
{
    EndScene();
}

AkBool SceneComputeShader::BeginScene()
{
    using namespace std;

    GRenderer->SetCameraPosition(0.0f, 0.0f, -2.0f);
    GRenderer->RotateYawPitchRollCamera(0.0f, 0.0f, 0.0f);

    Vector3 pRainbow[] =
    {
        {1.0f, 0.0f, 0.0f},  // Red
        {1.0f, 0.65f, 0.0f}, // Orange
        {1.0f, 1.0f, 0.0f},  // Yellow
        {0.0f, 1.0f, 0.0f},  // Green
        {0.0f, 0.0f, 1.0f},  // Blue
        {0.3f, 0.0f, 0.5f},  // Indigo
        {0.5f, 0.0f, 1.0f}   // Violet/Purple
    };

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<AkF32> dp(-1.0f, 1.0f);
    uniform_int_distribution<size_t> dc(0, _countof(pRainbow) - 1);
    for (auto& p : _pParticles) {
        p.vPosition = Vector3(dp(gen), dp(gen), 1.0f);
        p.vColor = Vector3(1.0f, 0.8f, 0.0f);
        // p.fRadius = (dp(gen) + 1.3f) * 0.02f;
        p.fLife = -1.0f;
    }

    _pParticle = GRenderer->CreateParticle();
    _pDBHandle = _pParticle->CreateBasicParticleBuffer(_pParticles, _countof(_pParticles));

    _pSpriteTexHandle = GRenderer->CreateTextureFromFile(L"../../assets/flare0.dds", AK_TRUE);
    _pParticle->SetTexture(_pSpriteTexHandle);

    return AK_TRUE;
}

AkBool SceneComputeShader::EndScene()
{
    if (_pSpriteTexHandle)
    {
        GRenderer->DestroyTexture(_pSpriteTexHandle);
        _pSpriteTexHandle = nullptr;
    }
    if (_pDBHandle)
    {
        _pParticle->DestroyBasicParticleBuffer(_pDBHandle);
        _pDBHandle = nullptr;
    }
    if (_pParticle)
    {
        _pParticle->Release();
        _pParticle = nullptr;
    }

    return AK_TRUE;
}

void SceneComputeShader::Update()
{
    using namespace std;

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<AkF32> randomTheta(-3.141592f, 3.141592f);
    uniform_real_distribution<AkF32> randomSpeed(1.5f, 2.0f);
    uniform_real_distribution<AkF32> randomLife(0.0f, 1.0f);

    AkU32 uNewCount = 20;
    for (auto& p : _pParticles)
    {
        if (p.fLife < 0.0f && uNewCount > 0)
        {
            const AkF32 fTheta = randomTheta(gen);

            p.vPosition = Vector3(0.0f);
            p.vVelocity = Vector3(cos(fTheta), -sin(fTheta), 0.0f) * randomSpeed(gen) * 0.3f;
            p.fLife = randomLife(gen) * 0.8f;
            p.fRadius = randomLife(gen) * 1.0f;
            uNewCount--;
        }
    }

    const Vector3 buoyancy = Vector3(0.0f, 2.0f, 0.0f);
    const Vector3 gravity = Vector3(0.0f, -9.8f, 0.0f);
    const float cor = 0.5f; // Coefficient Of Restitution
    const float groundHeight = -10.0f;

    for (auto& p : _pParticles) {

        if (p.fLife < 0.0f) // 수명이 다했다면 무시
            continue;

        p.vVelocity = p.vVelocity + buoyancy * DT;
        p.vPosition += p.vVelocity * DT;
        p.fLife -= DT;
    }

    GRenderer->UpdateDynamicDefaultBuffer(_pDBHandle, _pParticles);
}

void SceneComputeShader::FinalUpdate()
{
}

void SceneComputeShader::Render()
{
    GRenderer->RenderParticle(_pParticle, _pDBHandle);
}

void SceneComputeShader::RenderShadow()
{
}
