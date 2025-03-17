#include "pch.h"
#include "Particle.h"

/*
===========
Particle
===========
*/

Particle::Particle(const wchar_t* wcFilename)
{
	if (!Initialize(wcFilename))
	{
		__debugbreak();
	}
}

Particle::~Particle()
{
	CleanUp();
}

AkBool Particle::Initialize(const wchar_t* wcFilename)
{
	_pTexHandle = GRenderer->CreateTextureFromFile(wcFilename, AK_TRUE);

	return AK_TRUE;
}

void Particle::Render()
{
	GRenderer->RenderParticle(_pParticle, _pDBHandle);
}

void Particle::Play(const Vector3* pPos)
{
	_bIsPlay = AK_TRUE;
	_pTransform->SetPosition(pPos);
}

void Particle::Stop()
{
}

void Particle::Pause()
{
	_bIsPlay = AK_FALSE;
}

void Particle::Resume()
{
	_bIsPlay = AK_TRUE;
}

void Particle::CleanUp()
{
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
}

/*
===========
Spark
===========
*/

Spark::Spark(const wchar_t* wcFilename, ParticleInfo_t* pInfo)
	: Particle(wcFilename)
{
	if (!Initialize(pInfo))
	{
		__debugbreak();
	}
}

Spark::~Spark()
{
	CleanUp();
}

AkBool Spark::Initialize(ParticleInfo_t* pInfo)
{
	_pInfo = pInfo;

	CreateParticles();

	return AK_TRUE;
}

void Spark::Render()
{
	Particle::Render();
}

void Spark::CreateParticles()
{
	_pVertices = new VertexParticle_t[MAX_COUNT];
	_pParticle = GRenderer->CreateParticle();
	_pDBHandle = _pParticle->CreateBasicParticleBuffer(_pVertices, MAX_COUNT);
	_pParticle->SetTexture(_pTexHandle);
}

void Spark::Update()
{
	if (!_bIsPlay)
	{
		return;
	}

	fTime += DT;

	if (fTime > fDuration)
	{
		if (!_pInfo->bLoop)
		{
			Stop();
		}
		else
		{
			Play(&_pTransform->GetPosition());
		}
	}
}

void Spark::UpdateEditor()
{
}

void Spark::Play(const Vector3* pPos)
{
	Particle::Play(pPos);
	UpdateParticle();
	GRenderer->UpdateDynamicVertexBuffer(_pDBHandle, _pVertices);
}

void Spark::UpdateParticle()
{
	fTime = 0.0f;
	fDuration = _pInfo->fDuration;
	_uParticleCount = _pInfo->uCount;

	for (AkU32 i = 0; i < _pInfo->uCount; i++)
	{
		// Life Time.
		{
			AkF32 fMin = _pInfo->vStartLifeTime.x;
			AkF32 fMax = _pInfo->vStartLifeTime.y;
			AkF32 fValue = Random(fMin, fMax);
			_pVertices[i].fStartLifeTime = fValue;
		}
		// Speed.
		{
			AkF32 fMin = _pInfo->vStartSpeed.x;
			AkF32 fMax = _pInfo->vStartSpeed.y;
			AkF32 fValue = Random(fMin, fMax);
			_pVertices[i].fSpeed = fValue;
		}
		// Size.
		{
			AkF32 fMin = _pInfo->vStartSize.x;
			AkF32 fMax = _pInfo->vStartSize.y;
			AkF32 fValue = Random(fMin, fMax);
			_pVertices[i].vSize = Vector2(fValue, fValue);
		}
		// Direction.
		{
			SetDirectionByShape(_pInfo->iShape, i);
		}
		// Color.
		Vector4 vColor0 = _pInfo->vStartColor[0];
		Vector4 vColor1 = _pInfo->vStartColor[1];
		Vector4 vColor =
		{
			(vColor1.x - vColor0.x) * Random(0.0f, 1.0f) + vColor.x,
			(vColor1.y - vColor0.y) * Random(0.0f, 1.0f) + vColor.y,
			(vColor1.z - vColor0.z) * Random(0.0f, 1.0f) + vColor.z,
			1.0f
		};

		_pVertices[i].vColor = vColor;
	}

	fColorOverLifeTime = _pInfo->vColorOverLifeTime;
}

void Spark::SetDirectionByShape(AkI32 iShape, AkI32 i)
{
	AkF32 fMin = _pInfo->vStartRadius.x;
	AkF32 fMax = _pInfo->vStartRadius.y;
	AkF32 fRadius = Random(fMin, fMax);

	Vector3 vDir = _pTransform->Front() * fRadius;

	switch (iShape)
	{
	case Spark::SHPERE:
	{
		Vector3 vRot = Vector3(0.0f);
		vRot.x = Random(0.0f, DirectX::XM_2PI);
		vRot.y = Random(0.0f, DirectX::XM_2PI);
		vRot.z = Random(0.0f, DirectX::XM_2PI);

		Matrix mRot = Matrix::CreateFromYawPitchRoll(vRot.x, vRot.y, vRot.z);
		_pVertices[i].vDirection = Vector3::Transform(vDir, mRot);
	}
	break;
	case Spark::CIRCLE:
	{
		AkF32 fR1 = Random(-1.0f, 1.0f);
		AkF32 fR2 = Random(-1.0f, 1.0f);
		vDir = (_pTransform->Front() * fR1 + _pTransform->Right() * fR2);
		vDir.Normalize();
		vDir *= fRadius;

		_pVertices[i].vDirection = vDir;
	}
	break;
	case  Spark::CONE:
	{

	}
	break;
	}
}

void Spark::CleanUp()
{
	if (_pVertices)
	{
		delete _pVertices;
		_pVertices = nullptr;
	}
}
