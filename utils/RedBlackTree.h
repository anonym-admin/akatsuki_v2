#pragma once

/*
==================
Red Black Tree
==================
*/

struct RbTreeNode_t
{
	RbTreeNode_t* pParent = nullptr;
	RbTreeNode_t* pLeft = nullptr;
	RbTreeNode_t* pRight = nullptr;

	enum {
		RED,
		BLACK
	} eColor;

	void* pKey = nullptr;
	void* pData = nullptr;
};

void RBT_DestroyTree(RbTreeNode_t* pTree);
RbTreeNode_t* RBT_CreateNode(void* pKey, void* pData);
void RBT_DestroyNode(RbTreeNode_t* pNode);
RbTreeNode_t* RBT_SearchNode(RbTreeNode_t* pTree, void* pKey);
void RBT_InsertNode(RbTreeNode_t** ppTree, RbTreeNode_t* pNewNode);
void RBT_InsertNodeHelper(RbTreeNode_t** ppTree, RbTreeNode_t* pNewNode);
void RBT_RotateRight(RbTreeNode_t** ppRoot, RbTreeNode_t* pParent);
void RBT_RotateLeft(RbTreeNode_t** ppRoot, RbTreeNode_t* pParent);
void RBT_RebuildAfterInsert(RbTreeNode_t** ppRoot, RbTreeNode_t* pXNode);

void RBT_PrintTree(RbTreeNode_t* pNode);
void RBT_Test();