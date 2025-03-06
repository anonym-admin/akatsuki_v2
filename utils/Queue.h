#pragma once

#include "LinkedList.h"

/*
============
Queue
============
*/

struct Queue_t
{
	int iCapacity;
	int iFront;
	int iRear;
	unsigned char** pDataList;
};

void CQ_CreateQueue(Queue_t** ppOutQueue, int iCapacity, int iDataTypeSize);
void CQ_DestroyQueue(Queue_t* pQueue);
void CQ_Enqueue(Queue_t* pQueue, void* pData);
void* CQ_Dequeue(Queue_t* pQueue);
int CQ_GetSize(Queue_t* pQueue);
bool CQ_IsEmpty(Queue_t* pQueue);
bool CQ_IsFull(Queue_t* pQueue);

void CQ_Test();