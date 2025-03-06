#pragma once

#include "AkMeshData.h"

/*
==========
AABB
==========
*/

struct AkBox_t
{
	Vector3 vMin = Vector3(AK_MAX_F32);
	Vector3 vMax = Vector3(-AK_MAX_F32);
};

void Expand(AkBox_t* pAABB, const Vector3* pPoint);

/*
==============
Triangle
==============
*/

struct AkTriangle_t
{
	Vector3 vP[3] = {};
	Vector3 vNormal = Vector3(0.0f);
};

Vector3 Centroid(AkTriangle_t* pTri);

/*
=======
Plane
=======
*/

struct AkPlane_t
{
	Vector4 vPlane;
};

/*
==========
AkSphere
==========
*/

struct AkSphere_t
{
	AkF32 fRadius = 1.0f;
	Vector3 vCenter = Vector3(0.0f);
};

/*
==========
AkFrustum
==========
*/

struct AkFrustum_t
{
	AkPlane_t vPlane[6] = {};
};

AkFrustum_t* CreateFrustum(struct IRenderer* pRenderer);
void UpdateFrustum(struct IRenderer* pRenderer, AkFrustum_t* pFrustum);
void DestroyFrustum(AkFrustum_t* pFrustum);

/*
===================
Intersect Function
===================
*/

AkBool IntersectBoxToBox(AkBox_t* pMe, AkBox_t* pOther);
AkBool IntersectSphereToTriangle(AkSphere_t* pSphere, AkTriangle_t* pTri);
AkBool IntersectSphereToBox(AkSphere_t* pSphere, AkBox_t* pBox);
AkBool IntersectSphereToSphere(AkSphere_t* pSphereA, AkSphere_t* pSphereB);
AkBool IntersectFrustumToBox(AkFrustum_t* pFrustum, AkBox_t* pBox);