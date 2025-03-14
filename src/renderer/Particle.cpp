#include "pch.h"
#include "Particle.h"

/*
==============
Particle
==============
*/

FParticle::FParticle()
{
}

FParticle::~FParticle()
{
	CleanUp();
}

AkBool FParticle::Initialize(FRenderer* pRenderer)
{
	_pRenderer = pRenderer;
	return AK_TRUE;
}

AkBool FParticle::CreateParticleBuffer(VertexSize_t* pVertices, AkU32 uVerticeNum)
{
	return AkBool();
}

void FParticle::SetTexture(const wchar_t* wcFilaname)
{
}

HRESULT __stdcall FParticle::QueryInterface(REFIID riid, void** ppvObject)
{
	return E_NOTIMPL;
}

ULONG __stdcall FParticle::AddRef(void)
{
	return 0;
}

ULONG __stdcall FParticle::Release(void)
{
	return 0;
}

void FParticle::Draw(AkU32 uThreadIndex, ID3D12GraphicsCommandList* pCmdList)
{
}

void FParticle::CleanUp()
{
}
