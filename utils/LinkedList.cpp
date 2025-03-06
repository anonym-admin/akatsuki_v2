#include "pch.h"
#include "LinkedList.h"

/*
=====================
Linked List C style
=====================
*/

void LL_PushFront(List_t** ppHead, List_t** ppTail, List_t* pNew)
{
	if (!*ppHead)
	{
		*ppTail = *ppHead = pNew;
		(*ppHead)->pNext = nullptr;
		(*ppHead)->pPrev = nullptr;
	}
	else
	{
#ifdef _DEBUG
		if (*ppHead == pNew)
		{
			__debugbreak();
		}
#endif
		pNew->pNext = *ppHead;
		(*ppHead)->pPrev = pNew;
		(*ppHead) = pNew;
		pNew->pPrev = nullptr;
	}
}

void LL_PushBack(List_t** ppHead, List_t** ppTail, List_t* pNew)
{
	if (!*ppHead)
	{
		*ppTail = *ppHead = pNew;
		(*ppHead)->pNext = nullptr;
		(*ppHead)->pPrev = nullptr;

	}
	else
	{
		pNew->pPrev = (*ppTail);
		(*ppTail)->pNext = pNew;
		(*ppTail) = pNew;
		pNew->pNext = nullptr;
	}
}

void LL_Delete(List_t** ppHead, List_t** ppTail, List_t* pDel)
{
	List_t* pPrev = pDel->pPrev;
	List_t* pNext = pDel->pNext;

	if (pDel->pPrev)
	{
		pDel->pPrev->pNext = pDel->pNext;
	}
	else
	{
#ifdef _DEBUG
		if (pDel != *ppHead)
		{
			__debugbreak();
		}
#endif
		// Del Node => Head
		(*ppHead) = pNext;
	}

	if (pDel->pNext)
	{
		pDel->pNext->pPrev = pDel->pPrev;
	}
	else
	{
#ifdef _DEBUG
		if (pDel != *ppTail)
		{
			__debugbreak();
		}
#endif
		// Del Node => Tail
		(*ppTail) = pPrev;
	}

	pDel->pNext = nullptr;
	pDel->pPrev = nullptr;
}
