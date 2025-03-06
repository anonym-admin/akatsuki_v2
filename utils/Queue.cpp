#include "pch.h"
#include "Queue.h"
#include <stdlib.h>
#include <stdio.h>

/*
============
Queue
============
*/

void CQ_CreateQueue(Queue_t** ppOutQueue, int iCapacity, int iDataTypeSize)
{
    *ppOutQueue = (Queue_t*)malloc(sizeof(Queue_t));

    (*ppOutQueue)->pDataList = (unsigned char**)malloc(iDataTypeSize * (iCapacity + 1));

    (*ppOutQueue)->iCapacity = iCapacity;
    (*ppOutQueue)->iFront = 0;
    (*ppOutQueue)->iRear = 0;
}

void CQ_DestroyQueue(Queue_t* pQueue)
{
    if (pQueue)
    {
        free(pQueue->pDataList);
        free(pQueue);
    }
}

void CQ_Enqueue(Queue_t* pQueue, void* pData)
{
    int iPos = 0;

    if (pQueue->iRear == pQueue->iCapacity)
    {
        iPos = pQueue->iRear;
        pQueue->iRear = 0;
    }
    else
    {
        iPos = pQueue->iRear++;
    }

    pQueue->pDataList[iPos] = (unsigned char*)pData;
}

void* CQ_Dequeue(Queue_t* pQueue)
{
    int iPos = pQueue->iFront;

    if (pQueue->iFront == pQueue->iCapacity)
    {
        pQueue->iFront = 0;
    }
    else
    {
        pQueue->iFront++;
    }

    return pQueue->pDataList[iPos];
}

int CQ_GetSize(Queue_t* pQueue)
{
    if (pQueue->iFront <= pQueue->iRear)
    {
        return (pQueue->iRear - pQueue->iFront);
    }
    else
    {
        return (pQueue->iRear + (pQueue->iCapacity - pQueue->iFront) + 1);
    }
}

bool CQ_IsEmpty(Queue_t* pQueue)
{
    return (pQueue->iFront == pQueue->iRear);
}

bool CQ_IsFull(Queue_t* pQueue)
{
    if (pQueue->iFront < pQueue->iRear)
    {
        return (pQueue->iRear - pQueue->iFront) == pQueue->iCapacity;
    }
    else
    {
        return (pQueue->iRear + 1) == pQueue->iFront;
    }
}

void CQ_Test()
{
    Queue_t* pQueue = nullptr;

    CQ_CreateQueue(&pQueue, 16, sizeof(int));

    CQ_Enqueue(pQueue, (void*)5);
    CQ_Enqueue(pQueue, (void*)4);
    CQ_Enqueue(pQueue, (void*)3);
    CQ_Enqueue(pQueue, (void*)2);
    CQ_Enqueue(pQueue, (void*)1);

    while (!CQ_IsEmpty(pQueue))
    {
        void* pData = CQ_Dequeue(pQueue);

        printf("%d\n", (int)pData);
    }

    CQ_DestroyQueue(pQueue);
}
