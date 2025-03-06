#pragma once

class Collider;
class Actor;
struct AkTriangle_t;
struct AkFrustum_t;

/*
=============
KDTree Node
=============
*/

struct KDTreeNode_t
{
	Collider* pBox = nullptr;
	KDTreeNode_t* pLeft = nullptr;
	KDTreeNode_t* pRight = nullptr;
	Collider* pTri = nullptr;
	AkU32 uTriNum = 0;

	// For Render.
	Matrix mTransform = Matrix();
};

KDTreeNode_t* BuildKDTree(Actor* pOwner, Collider** pTriList, AkU32 uTriNum, AkU32 uDepth);
void DestroyKDTree(KDTreeNode_t* pNode);
void RenderKDTreeNode(KDTreeNode_t* pKDTreeNode, IRenderer* pRenderer, IMeshObject* pBoxMesh);
