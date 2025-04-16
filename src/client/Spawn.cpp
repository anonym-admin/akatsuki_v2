#include "pch.h"
#include "Spawn.h"

AkI32 Spawn::sm_iID;

Spawn::Spawn()
{
	ID = sm_iID++;
}

Spawn::~Spawn()
{
	CleanUp();
}

void Spawn::CleanUp()
{
}
