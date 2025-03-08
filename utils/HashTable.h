#pragma once

#include "LinkedList.h"

struct HASH_TABLE_BUCKET_DESC
{
	List_t* pBucketHead = nullptr;
	List_t* pBucketTail = nullptr;
	unsigned int uListSize = 0;
};

struct Bucket_t
{
	const void* pData = nullptr;
	HASH_TABLE_BUCKET_DESC* pBucketDesc = nullptr;
	List_t tLink = {};
	unsigned int uKeyLength = 0;
	char pKey[1] = {};
};

struct HashTable_t
{
	HASH_TABLE_BUCKET_DESC* pBucketDesc = nullptr;
	unsigned int uDataNum = 0;
	unsigned int uMaxKeyLength = 0;
	unsigned int uMaxBucketNum = 0;
	unsigned int uMaxDataNum = 0;
};

HashTable_t* HT_CreateHashTable(unsigned int uMaxBucketNum, unsigned int uMaxKeyLength, unsigned int uMaxDataNum);
void* HT_Insert(HashTable_t* pHashTable, const void* pData, const void* pKey, unsigned int uKeyLength);
unsigned int HT_Find(HashTable_t* pHashTable, void** ppDataList, unsigned int uGetItemNum, const void* pKey, unsigned int uKeyLength);
unsigned int HT_FindAll(HashTable_t* pHashTable, void** ppOutDataList, unsigned int uMaxItemNum, bool* pOutInSufficient);
void HT_Delete(HashTable_t* pHashTable, const void* pSearchHandle);
void HT_DeleteAll(HashTable_t* pHashTable);
void HT_DestroyHashTable(HashTable_t* pHashTable);
unsigned int HT_GetMaxBucketNum(HashTable_t* pHashTable);
unsigned int HT_GetDataNum(HashTable_t* pHashTable);
unsigned int HT_GetKeyPtrAndSize(void** ppOutKeyPtr, const void* pSearchHandle);
void HT_Check(HashTable_t* pHashTable);