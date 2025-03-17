#include "pch.h"
#include "MuzzleFlash.h"
#include <random>

/*
==================
Muzzle Flash
==================
*/


MuzzleFlash::MuzzleFlash(const wchar_t* wcTexFilename)
{
    if (!Initialize(wcTexFilename))
    {
        __debugbreak();
    }
}

MuzzleFlash::~MuzzleFlash()
{
    CleanUp();
}

AkBool MuzzleFlash::Initialize(const wchar_t* wcTexFilename)
{
    CreateTransform();

    _pTexHandle = GRenderer->CreateTextureFromFile(wcTexFilename, AK_TRUE);

    CreateParticle();

    return AK_TRUE;
}

void MuzzleFlash::Update()
{
}

void MuzzleFlash::UpdateEditor()
{
}

void MuzzleFlash::Render()
{
    GRenderer->RenderParticle(_pParticle, _pDBHandle);
}

void MuzzleFlash::Play(const Vector3* pPos)
{
    UpdateParticle();
}

void MuzzleFlash::Stop()
{
}

void MuzzleFlash::CleanUp()
{
    if (_pTexHandle)
    {
        GRenderer->DestroyTexture(_pTexHandle);
        _pTexHandle = nullptr;
    }
    if (_pTransform)
    {
        delete _pTransform;
        _pTransform = nullptr;
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
}

void MuzzleFlash::CreateTransform()
{
    _pTransform = new Transform;
}

void MuzzleFlash::CreateParticle()
{
    using namespace std;

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
        p.vColor = Vector3(1.0f, 0.8f, 0.0f);
        p.fLife = -1.0f;
    }

    _pParticle = GRenderer->CreateParticle();
    _pDBHandle = _pParticle->CreateBasicParticleBuffer(_pParticles, _countof(_pParticles));

    _pParticle->SetTexture(_pTexHandle);
}

void MuzzleFlash::UpdateParticle()
{
    using namespace std;

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<AkF32> randomTheta(-3.141592f, 3.141592f);
    uniform_real_distribution<AkF32> randomSpeed(1.5f, 2.0f);
    uniform_real_distribution<AkF32> randomLife(0.0f, 1.0f);

    AkU32 uNewCount = 1;
    for (auto& p : _pParticles)
    {
        if (p.fLife < 0.0f && uNewCount > 0)
        {
            const AkF32 fTheta = randomTheta(gen);

            p.vPosition = Vector3(3.0f, 1.5f, 1025.0f);
            p.vVelocity = Vector3(cos(fTheta), -sin(fTheta), 0.0f) * randomSpeed(gen) * 0.3f;
            p.fLife = randomLife(gen) * 0.8f;
            p.fRadius = randomLife(gen) * 1.0f;
            uNewCount--;
        }
    }

    const Vector3 buoyancy = Vector3(0.0f, 1.0f, 0.0f);
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
