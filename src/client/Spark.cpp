#include "pch.h"
#include "Spark.h"

/*
===========
Spark
===========
*/

Spark::Spark(const wchar_t* wcTexFilename, ParticleInfo_t* pInfo)
	: Particle(wcTexFilename)
{
	if (!Initialize(pInfo))
	{
		__debugbreak();
	}
}

Spark::Spark(const wchar_t* wcFxFilename)
{
	Load(wcFxFilename);
}

Spark::~Spark()
{
	CleanUp();
}

AkBool Spark::Initialize(ParticleInfo_t* pInfo)
{
	if (_pInfo)
	{
		delete _pInfo;
		_pInfo = nullptr;
	}

	_pInfo = pInfo;

	CreateParticles();

	return AK_TRUE;
}

void Spark::Render()
{
	printf("%lf\n", fTime);

	GRenderer->RenderParticle(_pParticle, &_pTransform->GetWorldTransform(), _pDBHandle, _uParticleCount, fTime, fDuration, &vStartSize, &vStartDirection, fSizeOverLifeTime, &vRotOverLifeTime, &vTotalColor, &vColorOverLifeTime);
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

	_pTransform->Update();

	fTime += DT;

	if (fTime > fDuration)
	{
		if (!_pInfo->bLoop)
		{
			Stop();
		}
		else
		{
			Vector3 vPos = _pTransform->GetPosition();
			Play(&vPos);
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

	GRenderer->UpdateDynamicDefaultBuffer(_pDBHandle, _pVertices);
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
			(vColor1.x - vColor0.x) * Random(0.0f, 1.0f) + vColor0.x,
			(vColor1.y - vColor0.y) * Random(0.0f, 1.0f) + vColor0.y,
			(vColor1.z - vColor0.z) * Random(0.0f, 1.0f) + vColor0.z,
			1.0f
		};

		_pVertices[i].vColor = vColor;
	}

	vColorOverLifeTime = _pInfo->vColorOverLifeTime;
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

		// printf("%lf %lf %lf\n", _pVertices[i].vDirection.x, _pVertices[i].vDirection.y, _pVertices[i].vDirection.z);
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

void Spark::Load(const wchar_t* wcFxFilename)
{
	DestroyParticles();

	FILE* fp = nullptr;
	_wfopen_s(&fp, wcFxFilename, L"rb");
	if (!fp) __debugbreak();

	Spark::ParticleInfo_t tInfo = {};
	AkI32 iSelectedItem = -1;
	fread(&tInfo, sizeof(Spark::ParticleInfo_t), 1, fp);
	fread(&iSelectedItem, sizeof(AkI32), 1, fp);

	_pInfo = new Spark::ParticleInfo_t;
	memcpy(_pInfo, &tInfo, sizeof(Spark::ParticleInfo_t));

	_iSelectName = iSelectedItem;

	std::wstring wcFullPath = L"../../assets/particle/" + ToWString(_pTexNames[_iSelectName]) + L".dds";
	Particle::Initialize(wcFullPath.c_str());

	CreateParticles();

	if (fp) fclose(fp);
}

void Spark::CleanUp()
{
	if (_pVertices)
	{
		delete _pVertices;
		_pVertices = nullptr;
	}
	if (_pInfo)
	{
		delete _pInfo;
		_pInfo = nullptr;
	}
}

void Spark::DestroyParticles()
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
}
