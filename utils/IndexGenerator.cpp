#include "pch.h"
#include "IndexGenerator.h"
#include <stdlib.h>
#include <string.h>

IndexGenerator_t* CreateIndexGenerator(unsigned int uMaxNum)
{
	IndexGenerator_t* pIndexGen = (IndexGenerator_t*)malloc(sizeof(IndexGenerator_t));
	pIndexGen->pIndexTable = (unsigned int*)malloc(sizeof(unsigned int) * uMaxNum);
	pIndexGen->uMaxNum = uMaxNum;
	pIndexGen->uAllocatedNum = 0;

	for (unsigned int i = 0; i < uMaxNum; i++)
	{
		pIndexGen->pIndexTable[i] = i;
	}

	return pIndexGen;
}

unsigned int AllocIndex(IndexGenerator_t* pIndexGen)
{
	unsigned int uIndex = -1;

	if (pIndexGen->uAllocatedNum >= pIndexGen->uMaxNum)
	{
		return uIndex;
	}

	uIndex = pIndexGen->pIndexTable[pIndexGen->uAllocatedNum];
	pIndexGen->uAllocatedNum++;

	return uIndex;
}

void FreeIndex(IndexGenerator_t* pIndexGen, unsigned int uIndex)
{
	if (!pIndexGen->uAllocatedNum)
	{
		__debugbreak();
		return;
	}

	pIndexGen->uAllocatedNum--;
	pIndexGen->pIndexTable[pIndexGen->uAllocatedNum] = uIndex;
}

void DestroyIndexGenerator(IndexGenerator_t* pIndexGen)
{
	if (pIndexGen)
	{
		if (pIndexGen->pIndexTable)
		{
			free(pIndexGen->pIndexTable);
			pIndexGen->pIndexTable = nullptr;
		}

		free(pIndexGen);
	}
}
