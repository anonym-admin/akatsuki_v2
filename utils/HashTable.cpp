#include "pch.h"
#include "HashTable.h"
#include <stdlib.h>
#include <string.h>

static unsigned int _CreateKey(const void* pKey, unsigned int uKeyLength, unsigned int uBuckNum)
{
	unsigned int uKeyData = 0;
	unsigned int uIndex = 0;

	const char* pEntry = (char*)pKey;
	if (uKeyLength & 0x00000001)
	{
		uKeyData += (unsigned int)(*(unsigned char*)pEntry);
		pEntry++;
		uKeyLength--;
	}
	if (!uKeyLength)
	{
		return uKeyData % uBuckNum;
	}

	if (uKeyLength & 0x00000002)
	{
		uKeyData += (unsigned int)(*(int*)pEntry);
		pEntry += 2;
		uKeyLength -= 2;
	}
	if (!uKeyLength)
	{
		return uKeyData % uBuckNum;
	}

	uKeyLength = (uKeyLength >> 2);

	for (unsigned int i = 0; i < uKeyLength; i++)
	{
		uKeyData += *(unsigned int*)pEntry;
		pEntry += 4;
	}

	uIndex = uKeyData % uBuckNum;

	return uIndex;
}

static Bucket_t* _CreateBucket(HashTable_t* pHashTable, const void* pData, const void* pKey, unsigned __int32 uKeyLength)
{
	unsigned int uBucketMemSize = (unsigned int)(sizeof(Bucket_t) - sizeof(char)) + pHashTable->uMaxKeyLength;
	Bucket_t* pBucket = (Bucket_t*)malloc(uBucketMemSize);
	memset(pBucket, 0, uBucketMemSize);

	unsigned int uIndex = _CreateKey(pKey, uKeyLength, pHashTable->uMaxBucketNum);
	HASH_TABLE_BUCKET_DESC* pBucketDesc = pHashTable->pBucketDesc + uIndex;
		
	pBucket->pData = pData;
	pBucket->pBucketDesc = pBucketDesc;
	pBucket->uKeyLength = uKeyLength;
	pBucket->tLink.pData = pBucket;
	pBucketDesc->uListSize++;

	memcpy(pBucket->pKey, pKey, uKeyLength);

	LL_PushBack(&pBucketDesc->pBucketHead, &pBucketDesc->pBucketTail, &pBucket->tLink);

	return pBucket;
}

HashTable_t* HT_CreateHashTable(unsigned int uMaxBucketNum, unsigned int uMaxKeyLength, unsigned int uMaxDataNum)
{
    HashTable_t* pHashTable = reinterpret_cast<HashTable_t*>(malloc(sizeof(HashTable_t)));
    pHashTable->uMaxBucketNum = uMaxBucketNum;
    pHashTable->uMaxKeyLength = uMaxKeyLength;
    pHashTable->uMaxDataNum = uMaxDataNum;
    pHashTable->uDataNum = 0;
    pHashTable->pBucketDesc = reinterpret_cast<HASH_TABLE_BUCKET_DESC*>(malloc(sizeof(HASH_TABLE_BUCKET_DESC) * uMaxBucketNum));
    memset(pHashTable->pBucketDesc, 0, sizeof(HASH_TABLE_BUCKET_DESC) * uMaxBucketNum);
   
    return pHashTable;
}

void* HT_Insert(HashTable_t* pHashTable, const void* pData, const void* pKey, unsigned int uKeyLength)
{
	void* pSearchHandle = nullptr;

	if (uKeyLength > pHashTable->uMaxKeyLength)
	{
		__debugbreak();
		return pSearchHandle;
	}

	Bucket_t* pBucket = _CreateBucket(pHashTable, pData, pKey, uKeyLength);

	pHashTable->uDataNum++;
	pSearchHandle = pBucket;

	return pSearchHandle;
}

unsigned int HT_Find(HashTable_t* pHashTable, void** ppDataList, unsigned int uGetItemNum, const void* pKey, unsigned int uKeyLength)
{
	unsigned int uSelectedItemNum = 0;
	unsigned int uIndex = _CreateKey(pKey, uKeyLength, pHashTable->uMaxBucketNum);
	HASH_TABLE_BUCKET_DESC* pBucketDesc = pHashTable->pBucketDesc + uIndex;
	
	List_t* pCur = pBucketDesc->pBucketHead;

	Bucket_t* pBucket = nullptr;
	while (pCur)
	{
		if (!uGetItemNum)
		{
			break;
		}

		pBucket = reinterpret_cast<Bucket_t*>(pCur->pData);

		if (pBucket->uKeyLength != uKeyLength)
		{
			pCur = pCur->pNext;
			continue;
		}

		if (memcmp(pBucket->pKey, pKey, uKeyLength))
		{
			pCur = pCur->pNext;
			continue;
		}

		ppDataList[uSelectedItemNum++] = (void*)pBucket->pData;
		uGetItemNum--;

		pCur = pCur->pNext;
	}

	return uSelectedItemNum;
}

unsigned int HT_FindAll(HashTable_t* pHashTable, void** ppOutDataList, unsigned int uMaxItemNum, bool* pOutInSufficient)
{
	Bucket_t* pBucket = nullptr;
	List_t* pCur = nullptr;

	*pOutInSufficient = false;
	unsigned int uItemNum = 0;

	bool bRet = false;
	for (unsigned int i = 0; i < pHashTable->uMaxBucketNum; i++)
	{
		pCur = pHashTable->pBucketDesc[i].pBucketHead;
		while (pCur)
		{
			pBucket = (Bucket_t*)pCur->pData;

			if (uItemNum >= pHashTable->uDataNum)
			{
				*pOutInSufficient = true;
				bRet = true;
			}

			if (bRet)
			{
				break;
			}

			ppOutDataList[uItemNum] = (void*)pBucket->pData;
			uItemNum++;

			pCur = pCur->pNext;
		}

		if (bRet)
		{
			break;
		}
	}

	return uItemNum;
}

void HT_Delete(HashTable_t* pHashTable, const void* pSearchHandle)
{
	Bucket_t* pBucket = (Bucket_t*)pSearchHandle;
	HASH_TABLE_BUCKET_DESC* pBucketDesc = pBucket->pBucketDesc;

	LL_Delete(&pBucketDesc->pBucketHead, &pBucketDesc->pBucketTail, &pBucket->tLink);

	pBucketDesc->uListSize--;

	free(pBucket);

	pHashTable->uDataNum--;
}

void HT_DeleteAll(HashTable_t* pHashTable)
{
	Bucket_t* pBucket = nullptr;
	for (unsigned int i = 0; i < pHashTable->uMaxBucketNum; i++)
	{
		while (pHashTable->pBucketDesc[i].pBucketHead)
		{
			pBucket = (Bucket_t*)pHashTable->pBucketDesc[i].pBucketHead->pData;
			HT_Delete(pHashTable, pBucket);
		}
	}
}

void HT_DestroyHashTable(HashTable_t* pHashTable)
{
	HT_Check(pHashTable);

	HT_DeleteAll(pHashTable);

    if (pHashTable)
    {
		if (pHashTable->pBucketDesc)
		{
			free(pHashTable->pBucketDesc);
			pHashTable->pBucketDesc = nullptr;
		}

        free(pHashTable);
    }
}

unsigned int HT_GetMaxBucketNum(HashTable_t* pHashTable)
{
	return pHashTable->uMaxBucketNum;
}

unsigned int HT_GetDataNum(HashTable_t* pHashTable)
{
	return pHashTable->uDataNum;
}

unsigned int HT_GetKeyPtrAndSize(void** ppOutKeyPtr, const void* pSearchHandle)
{
	*ppOutKeyPtr = ((Bucket_t*)pSearchHandle)->pKey;
	unsigned int uSize = ((Bucket_t*)pSearchHandle)->uKeyLength;
	return uSize;
}

void HT_Check(HashTable_t* pHashTable)
{
	if (pHashTable->uDataNum)
	{
		__debugbreak();
	}
}
