#include "pch.h"
#include "RedBlackTree.h"
#include <stdlib.h>

/*
==================
Red Black Tree
==================
*/

extern RbTreeNode_t* g_pNil;

RbTreeNode_t* RBT_CreateNode(void* pKey, void* pData)
{
    RbTreeNode_t* pNode = (RbTreeNode_t*)malloc(sizeof(RbTreeNode_t));
    pNode->pParent = nullptr;
    pNode->pLeft = nullptr;
    pNode->pRight = nullptr;
    pNode->pKey = pKey;
    pNode->pData = pData;
    pNode->eColor = RbTreeNode_t::BLACK;

    return pNode;
}

void RBT_DestroyNode(RbTreeNode_t* pNode)
{
    if (pNode)
    {
        free(pNode);
    }
}

void RBT_DestroyTree(RbTreeNode_t* pTree)
{
    if (pTree->pRight != g_pNil)
    {
        RBT_DestroyTree(pTree->pRight);
    }
    
    if (pTree->pLeft != g_pNil)
    {
        RBT_DestroyTree(pTree->pLeft);
    }

    pTree->pLeft = g_pNil;
    pTree->pRight = g_pNil;

    RBT_DestroyNode(pTree);
}

RbTreeNode_t* RBT_SearchNode(RbTreeNode_t* pTree, void* pKey)
{
    if (pTree == g_pNil || pTree == nullptr)
    {
        return nullptr;
    }

    unsigned __int64 u64SrcKey = (unsigned __int64)pTree->pKey;
    unsigned __int64 u64DestKey = (unsigned __int64)pKey;
    if (u64SrcKey > u64DestKey)
    {
        return RBT_SearchNode(pTree->pLeft, pKey);
    }
    else if (u64SrcKey < u64DestKey)
    {
        return RBT_SearchNode(pTree->pRight, pKey);
    }
    else
    {
        return pTree;
    }
}

void RBT_InsertNode(RbTreeNode_t** ppTree, RbTreeNode_t* pNewNode)
{
    RBT_InsertNodeHelper(ppTree, pNewNode);

    pNewNode->eColor = RbTreeNode_t::RED;
    pNewNode->pLeft = g_pNil;
    pNewNode->pRight = g_pNil;

   
}

void RBT_InsertNodeHelper(RbTreeNode_t** ppTree, RbTreeNode_t* pNewNode)
{
    if (nullptr == (*ppTree))
    {
        (*ppTree) = pNewNode;
    }

    unsigned __int64 u64SrcKey = (unsigned __int64)(*ppTree)->pKey;
    unsigned __int64 u64DestKey = (unsigned __int64)pNewNode->pKey;
    if (u64SrcKey < u64DestKey)
    {
        if ((*ppTree)->pRight == g_pNil)
        {
            (*ppTree)->pRight = pNewNode;
            pNewNode->pParent = (*ppTree);
        }
        else
        {
            RBT_InsertNodeHelper(&(*ppTree)->pRight, pNewNode);
        }
    }
    else if (u64SrcKey > u64DestKey)
    {
        if ((*ppTree)->pLeft == g_pNil)
        {
            (*ppTree)->pLeft = pNewNode;
            pNewNode->pParent = (*ppTree);
        }
        else
        {
            RBT_InsertNodeHelper(&(*ppTree)->pLeft, pNewNode);
        }
    }
}

void RBT_RotateRight(RbTreeNode_t** ppRoot, RbTreeNode_t* pParent)
{
    RbTreeNode_t* pLeftChild = pParent->pLeft;

    pParent->pLeft = pLeftChild->pRight;

    if (pLeftChild->pRight != g_pNil)
    {
        pLeftChild->pRight->pParent = pParent;
    }

    pLeftChild->pParent = pParent->pParent;

    if (pParent->pParent == nullptr)
    {
        (*ppRoot) = pLeftChild;
    }
    else
    {
        if (pParent == pParent->pParent->pLeft)
        {
            pParent->pParent->pLeft = pLeftChild;
        }
        else
        {
            pParent->pParent->pRight = pLeftChild;
        }
    }

    pLeftChild->pRight = pParent;
    pParent->pParent = pLeftChild;
}

void RBT_RotateLeft(RbTreeNode_t** ppRoot, RbTreeNode_t* pParent)
{
    RbTreeNode_t* pRightChild = pParent->pRight;

    pParent->pLeft = pRightChild->pRight;

    if (pRightChild->pLeft != g_pNil)
    {
        pRightChild->pLeft->pParent = pParent;
    }

    pRightChild->pParent = pParent->pParent;

    if (pParent->pParent == nullptr)
    {
        (*ppRoot) = pRightChild;
    }
    else
    {
        if (pParent == pParent->pParent->pLeft)
        {
            pParent->pParent->pLeft = pRightChild;
        }
        else
        {
            pParent->pParent->pRight = pRightChild;
        }
    }

    pRightChild->pLeft = pParent;
    pParent->pParent = pRightChild;
}

void RBT_RebuildAfterInsert(RbTreeNode_t** ppRoot, RbTreeNode_t* pXNode)
{
    while (pXNode != (*ppRoot) && pXNode->pParent->eColor == RbTreeNode_t::RED)
    {
        if (pXNode->pParent == pXNode->pParent->pParent->pLeft)
        {
            RbTreeNode_t* pUncle = pXNode->pParent->pParent->pRight;
            if (pUncle->eColor == RbTreeNode_t::RED)
            {
                pXNode->pParent->eColor = RbTreeNode_t::BLACK;
                pUncle->eColor = RbTreeNode_t::BLACK;
                pXNode->pParent->pParent->eColor = RbTreeNode_t::RED;

                pXNode = pXNode->pParent->pParent;
            }
            else
            {
                if (pXNode == pXNode->pParent->pRight)
                {
                    pXNode = pXNode->pParent;
                    RBT_RotateLeft(ppRoot, pXNode);
                }

                pXNode->pParent->eColor = RbTreeNode_t::BLACK;
                pXNode->pParent->pParent->eColor = RbTreeNode_t::RED;

                RBT_RotateRight(ppRoot, pXNode->pParent->pParent);
            }
        }
        else
        {
            RbTreeNode_t* pUncle = pXNode->pParent->pParent->pLeft;
            if (pUncle->eColor == RbTreeNode_t::RED)
            {
                pXNode->pParent->eColor = RbTreeNode_t::BLACK;
                pUncle->eColor = RbTreeNode_t::BLACK;
                pXNode->pParent->pParent->eColor = RbTreeNode_t::RED;

                pXNode = pXNode->pParent->pParent;
            }
            else
            {
                if (pXNode == pXNode->pParent->pLeft)
                {
                    pXNode = pXNode->pParent;
                    RBT_RotateLeft(ppRoot, pXNode);
                }

                pXNode->pParent->eColor = RbTreeNode_t::BLACK;
                pXNode->pParent->pParent->eColor = RbTreeNode_t::RED;

                RBT_RotateLeft(ppRoot, pXNode->pParent->pParent);
            }
        }
    }

    (*ppRoot)->eColor = RbTreeNode_t::BLACK;
}

#include <stdio.h>

void RBT_PrintTree(RbTreeNode_t* pNode)
{
    if (pNode == nullptr || pNode == g_pNil)
    {
        return;
    }

    printf("%llu %p\n", (unsigned long long)pNode->pKey, pNode->pData);

    RBT_PrintTree(pNode->pLeft);
    RBT_PrintTree(pNode->pRight);
}

void RBT_Test()
{
    RbTreeNode_t* pRbTree = nullptr;
    RbTreeNode_t* pNode = nullptr;

    g_pNil = RBT_CreateNode(nullptr, nullptr);
    g_pNil->eColor = RbTreeNode_t::BLACK;

    int a0 = 1;
    RBT_InsertNode(&pRbTree, RBT_CreateNode((void*)1, &a0));

    int a1 = 1;
    RBT_InsertNode(&pRbTree, RBT_CreateNode((void*)2, &a1));

    int a2 = 1;
    RBT_InsertNode(&pRbTree, RBT_CreateNode((void*)3, &a2));

    int a3 = 1;
    RBT_InsertNode(&pRbTree, RBT_CreateNode((void*)4, &a3));

    int a4 = 1;
    RBT_InsertNode(&pRbTree, RBT_CreateNode((void*)5, &a4));

    int a5 = 1;
    RBT_InsertNode(&pRbTree, RBT_CreateNode((void*)6, &a5));

    int a6 = 1;
    RBT_InsertNode(&pRbTree, RBT_CreateNode((void*)7, &a6));

    // Search
    pNode = RBT_SearchNode(pRbTree, (void*)5);

    printf("%p\n", pNode->pData);

    RBT_PrintTree(pRbTree);

    RBT_DestroyTree(pRbTree);
}
