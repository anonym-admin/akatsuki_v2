#pragma once

struct IndexGenerator_t
{
	unsigned int* pIndexTable = nullptr;
	unsigned int uAllocatedNum = 0;
	unsigned int uMaxNum = 0;
};

IndexGenerator_t* CreateIndexGenerator(unsigned int uMaxNum);
unsigned int AllocIndex(IndexGenerator_t* pIndexGen);
void FreeIndex(IndexGenerator_t* pIndexGen, unsigned int uIndex);
void DestroyIndexGenerator(IndexGenerator_t* pIndexGen);