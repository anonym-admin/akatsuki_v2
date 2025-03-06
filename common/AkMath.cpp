#include "pch.h"
#include "AkMath.h"

/*
==========
AABB
==========
*/

void Expand(AkBox_t* pAABB, const Vector3* pPoint)
{
	pAABB->vMin.x = min(pAABB->vMin.x, pPoint->x);
	pAABB->vMin.y = min(pAABB->vMin.y, pPoint->y);
	pAABB->vMin.z = min(pAABB->vMin.z, pPoint->z);
	pAABB->vMax.x = max(pAABB->vMax.x, pPoint->x);
	pAABB->vMax.y = max(pAABB->vMax.y, pPoint->y);
	pAABB->vMax.z = max(pAABB->vMax.z, pPoint->z);
}

/*
==============
Triangle
==============
*/

Vector3 Centroid(AkTriangle_t* pTri)
{
	return (pTri->vP[0] + pTri->vP[1] + pTri->vP[2]) / 3.0f;
}

/*
==========
AkFrustum
==========
*/

AkFrustum_t* CreateFrustum(IRenderer* pRenderer)
{
    AkFrustum_t* pFrustum = new AkFrustum_t;

    pRenderer->GetFrustum(pFrustum->vPlane);

    //// For Debugging.
    //printf("%lf %lf %lf %lf\n", pFrustum->vPlane[0].vPlane.x, pFrustum->vPlane[0].vPlane.y, pFrustum->vPlane[0].vPlane.y, pFrustum->vPlane[0].vPlane.z);
    //printf("%lf %lf %lf %lf\n", pFrustum->vPlane[1].vPlane.x, pFrustum->vPlane[1].vPlane.y, pFrustum->vPlane[1].vPlane.y, pFrustum->vPlane[1].vPlane.z);
    //printf("%lf %lf %lf %lf\n", pFrustum->vPlane[2].vPlane.x, pFrustum->vPlane[2].vPlane.y, pFrustum->vPlane[2].vPlane.y, pFrustum->vPlane[2].vPlane.z);
    //printf("%lf %lf %lf %lf\n", pFrustum->vPlane[3].vPlane.x, pFrustum->vPlane[3].vPlane.y, pFrustum->vPlane[3].vPlane.y, pFrustum->vPlane[3].vPlane.z);

    return pFrustum;
}

void UpdateFrustum(IRenderer* pRenderer, AkFrustum_t* pFrustum)
{
    pRenderer->GetFrustum(pFrustum->vPlane);
}

void DestroyFrustum(AkFrustum_t* pFrustum)
{
    if (pFrustum)
    {
        delete pFrustum;
    }
}

/*
===================
Intersect Function
===================
*/


AkBool IntersectBoxToBox(AkBox_t* pMe, AkBox_t* pOther)
{
	AkBool bIntersect = (pMe->vMin.x <= pOther->vMax.x && pMe->vMax.x >= pOther->vMin.x) &&
		(pMe->vMin.y <= pOther->vMax.y && pMe->vMax.y >= pOther->vMin.y) &&
		(pMe->vMin.z <= pOther->vMax.z && pMe->vMax.z >= pOther->vMin.z);

	return bIntersect;
}

AkBool IntersectSphereToTriangle(AkSphere_t* pSphere, AkTriangle_t* pTri)
{
    using namespace DirectX;
    
    // Load the sphere.
    XMVECTOR vCenter = pSphere->vCenter;
    XMVECTOR vRadius = XMVectorReplicatePtr(&pSphere->fRadius);

    // Compute the plane of the triangle (has to be normalized).
    XMVECTOR N = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(pTri->vP[1], pTri->vP[0]), XMVectorSubtract(pTri->vP[2], pTri->vP[0])));

    // Assert that the triangle is not degenerate.
    assert(!XMVector3Equal(N, XMVectorZero()));

    // Find the nearest feature on the triangle to the sphere.
    XMVECTOR Dist = XMVector3Dot(XMVectorSubtract(vCenter, pTri->vP[0]), N);

    // If the center of the sphere is farther from the plane of the triangle than
    // the radius of the sphere, then there cannot be an intersection.
    XMVECTOR NoIntersection = XMVectorLess(Dist, XMVectorNegate(vRadius));
    NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Dist, vRadius));

    // Project the center of the sphere onto the plane of the triangle.
    XMVECTOR Point = XMVectorNegativeMultiplySubtract(N, Dist, vCenter);

    // Is it inside all the edges? If so we intersect because the distance
    // to the plane is less than the radius.
    XMVECTOR Intersection = DirectX::Internal::PointOnPlaneInsideTriangle(Point, pTri->vP[0], pTri->vP[1], pTri->vP[2]);

    // Find the nearest point on each edge.
    XMVECTOR RadiusSq = XMVectorMultiply(vRadius, vRadius);

    // Edge 0,1
    Point = DirectX::Internal::PointOnLineSegmentNearestPoint(pTri->vP[0], pTri->vP[1], vCenter);

    // If the distance to the center of the sphere to the point is less than
    // the radius of the sphere then it must intersect.
    Intersection = XMVectorOrInt(Intersection, XMVectorLessOrEqual(XMVector3LengthSq(XMVectorSubtract(vCenter, Point)), RadiusSq));

    // Edge 1,2
    Point = DirectX::Internal::PointOnLineSegmentNearestPoint(pTri->vP[1], pTri->vP[2], vCenter);

    // If the distance to the center of the sphere to the point is less than
    // the radius of the sphere then it must intersect.
    Intersection = XMVectorOrInt(Intersection, XMVectorLessOrEqual(XMVector3LengthSq(XMVectorSubtract(vCenter, Point)), RadiusSq));

    // Edge 2,0
    Point = DirectX::Internal::PointOnLineSegmentNearestPoint(pTri->vP[2], pTri->vP[0], vCenter);

    // If the distance to the center of the sphere to the point is less than
    // the radius of the sphere then it must intersect.
    Intersection = XMVectorOrInt(Intersection, XMVectorLessOrEqual(XMVector3LengthSq(XMVectorSubtract(vCenter, Point)), RadiusSq));

    return XMVector4EqualInt(XMVectorAndCInt(Intersection, NoIntersection), XMVectorTrueInt());
}

AkBool IntersectSphereToBox(AkSphere_t* pSphere, AkBox_t* pBox)
{
    using namespace DirectX;

    XMVECTOR SphereCenter = XMLoadFloat3(&pSphere->vCenter);
    XMVECTOR SphereRadius = XMVectorReplicatePtr(&pSphere->fRadius);

    Vector3 vBoxCenter = (pBox->vMax + pBox->vMin) * 0.5f;
    Vector3 vExtents = (pBox->vMax - pBox->vMin);

    XMVECTOR BoxCenter = XMLoadFloat3(&vBoxCenter);
    XMVECTOR BoxExtents = XMLoadFloat3(&vExtents);

    XMVECTOR BoxMin = XMVectorSubtract(BoxCenter, BoxExtents);
    XMVECTOR BoxMax = XMVectorAdd(BoxCenter, BoxExtents);

    // Find the distance to the nearest point on the box.
    // for each i in (x, y, z)
    // if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
    // else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

    XMVECTOR d = XMVectorZero();

    // Compute d for each dimension.
    XMVECTOR LessThanMin = XMVectorLess(SphereCenter, BoxMin);
    XMVECTOR GreaterThanMax = XMVectorGreater(SphereCenter, BoxMax);

    XMVECTOR MinDelta = XMVectorSubtract(SphereCenter, BoxMin);
    XMVECTOR MaxDelta = XMVectorSubtract(SphereCenter, BoxMax);

    // Choose value for each dimension based on the comparison.
    d = XMVectorSelect(d, MinDelta, LessThanMin);
    d = XMVectorSelect(d, MaxDelta, GreaterThanMax);

    // Use a dot-product to square them and sum them together.
    XMVECTOR d2 = XMVector3Dot(d, d);

    return XMVector3LessOrEqual(d2, XMVectorMultiply(SphereRadius, SphereRadius));
}

AkBool IntersectSphereToSphere(AkSphere_t* pSphereA, AkSphere_t* pSphereB)
{
	return AkBool();
}

AkBool IntersectFrustumToBox(AkFrustum_t* pFrustum, AkBox_t* pBox)
{
    using namespace DirectX;

    Vector3 vBoxCenter = (pBox->vMax + pBox->vMin) * 0.5f;
    Vector3 vExtents = (pBox->vMax - pBox->vMin) * 0.5f;

    Vector4 vPoints[8] = {};
    vPoints[0] = Vector4(vBoxCenter.x - vExtents.x, vBoxCenter.y - vExtents.y, vBoxCenter.z - vExtents.z, 1.0f);
    vPoints[1] = Vector4(vBoxCenter.x + vExtents.x, vBoxCenter.y - vExtents.y, vBoxCenter.z - vExtents.z, 1.0f);
    vPoints[2] = Vector4(vBoxCenter.x - vExtents.x, vBoxCenter.y + vExtents.y, vBoxCenter.z - vExtents.z, 1.0f);
    vPoints[3] = Vector4(vBoxCenter.x - vExtents.x, vBoxCenter.y - vExtents.y, vBoxCenter.z + vExtents.z, 1.0f);
    vPoints[4] = Vector4(vBoxCenter.x + vExtents.x, vBoxCenter.y + vExtents.y, vBoxCenter.z - vExtents.z, 1.0f);
    vPoints[5] = Vector4(vBoxCenter.x + vExtents.x, vBoxCenter.y - vExtents.y, vBoxCenter.z + vExtents.z, 1.0f);
    vPoints[6] = Vector4(vBoxCenter.x - vExtents.x, vBoxCenter.y + vExtents.y, vBoxCenter.z + vExtents.z, 1.0f);
    vPoints[7] = Vector4(vBoxCenter.x + vExtents.x, vBoxCenter.y + vExtents.y, vBoxCenter.z + vExtents.z, 1.0f);

    for (AkU32 i = 0; i < 6; i++)
    {
        if (XMVectorGetX(XMPlaneDotCoord(pFrustum->vPlane[i].vPlane, vPoints[0])) >= 0.0f)
            continue;

        if (XMVectorGetX(XMPlaneDotCoord(pFrustum->vPlane[i].vPlane, vPoints[1])) >= 0.0f)
            continue;

        if (XMVectorGetX(XMPlaneDotCoord(pFrustum->vPlane[i].vPlane, vPoints[2])) >= 0.0f)
            continue;

        if (XMVectorGetX(XMPlaneDotCoord(pFrustum->vPlane[i].vPlane, vPoints[3])) >= 0.0f)
            continue;

        if (XMVectorGetX(XMPlaneDotCoord(pFrustum->vPlane[i].vPlane, vPoints[4])) >= 0.0f)
            continue;

        if (XMVectorGetX(XMPlaneDotCoord(pFrustum->vPlane[i].vPlane, vPoints[5])) >= 0.0f)
            continue;

        if (XMVectorGetX(XMPlaneDotCoord(pFrustum->vPlane[i].vPlane, vPoints[6])) >= 0.0f)
            continue;

        if (XMVectorGetX(XMPlaneDotCoord(pFrustum->vPlane[i].vPlane, vPoints[7])) >= 0.0f)
            continue;

        return AK_FALSE;
    }

    return AK_TRUE;
}

