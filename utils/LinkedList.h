#pragma once

/*
=====================
Linked List C style
=====================
*/

struct List_t
{
	List_t* pPrev = nullptr;
	List_t* pNext = nullptr;
	void* pData = nullptr;
};

void LL_PushFront(List_t** ppHead, List_t** ppTail, List_t* pNew);
void LL_PushBack(List_t** ppHead, List_t** ppTail, List_t* pNew);
void LL_Delete(List_t** ppHead, List_t** ppTail, List_t* pDel);
