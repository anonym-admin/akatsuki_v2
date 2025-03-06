#include "pch.h"
#include "Spawn.h"

Spawn::Spawn()
{
}

Spawn::~Spawn()
{
	CleanUp();
}

Spawn* Spawn::Clone()
{
	_uInstanceCount++;
	return nullptr;
}

void Spawn::CleanUp()
{
	AkU32 uRefCount = _uInstanceCount - 1;
	if (uRefCount)
	{
		return;
	}
}
