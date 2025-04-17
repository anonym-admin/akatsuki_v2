#include "pch.h"
#include "FrustumCulling.h"
#include "Scene.h"

FrustumCulling::FrustumCulling()
{
    if (!Initialize())
    {
        __debugbreak();
    }
}

FrustumCulling::~FrustumCulling()
{
}

AkBool FrustumCulling::Initialize()
{
    // Renderer 로 부터 Frustum Plane 을 attach.
    GRenderer->GetFrustum(_pPlane);

    return AK_TRUE;
}

AkU32 FrustumCulling::Process()
{
    // 매 프레임 마다 _pPlane attach.
    GRenderer->GetFrustum(_pPlane);

    AkU32 uTotalRenderObj = 0;
    AkU32 uRenderObj = 0;

    Scene* pScene = GSceneManager->GetCurrentScene();
    GameObjContainer_t** ppGameObjContainer = pScene->GetAllGameObject();
    for (AkU32 i = 0; i < (AkU32)GAME_OBJECT_GROUP_TYPE::COUNT; i++)
    {
        if (ppGameObjContainer[i])
        {
			List_t* pCur = ppGameObjContainer[i]->pGameObjHead;
			while (pCur != nullptr)
			{
                Actor* pObj = (Actor*)pCur->pData;
                BoxCollider* pBox = (BoxCollider*)pObj->GetCullingCollider();

                if (!pBox)
                {
                    pCur = pCur->pNext;
                    continue;
                }

                Vector3 vCenter = (pBox->GetMaxWorld() + pBox->GetMinWorld()) * 0.5f;
                AkF32 fRadiusX = (pBox->GetMaxWorld().x - pBox->GetMinWorld().x) * 0.5f;
                AkF32 fRadiusY = (pBox->GetMaxWorld().y - pBox->GetMinWorld().y) * 0.5f;
                AkF32 fRadiusZ = (pBox->GetMaxWorld().z - pBox->GetMinWorld().z) * 0.5f;
                AkBool bIntersect = AK_FALSE;
                if (!(bIntersect = CheckCube(vCenter.x, vCenter.y, vCenter.z, fRadiusX, fRadiusY, fRadiusZ)))
                {
                    pObj->Cull = AK_TRUE; // Rendering 되지 않는 Object
                }
                else
                {
                    pObj->Cull = AK_FALSE;
                    uRenderObj++;
                }

				pCur = pCur->pNext;
                uTotalRenderObj++;
			}
        }
    }

    _uCullingCount = uTotalRenderObj - uRenderObj;
    _uTotalRenderObj = uTotalRenderObj;
    
    return uRenderObj;
}

void FrustumCulling::Render()
{
}

void FrustumCulling::RenderGUI()
{
}

AkBool FrustumCulling::CheckCube(AkF32 fCenterX, AkF32 fCenterY, AkF32 fCenterZ, AkF32 fRadiusX, AkF32 fRadiusY, AkF32 fRadiusZ)
{
    using namespace DirectX;

    for (AkU32 i = 0; i < 6; i++)
    {
        if (XMVectorGetX(XMPlaneDotCoord(_pPlane[i], XMVectorSet(fCenterX - fRadiusX, fCenterY - fRadiusY, fCenterZ - fRadiusZ, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(_pPlane[i], XMVectorSet(fCenterX + fRadiusX, fCenterY - fRadiusY, fCenterZ - fRadiusZ, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(_pPlane[i], XMVectorSet(fCenterX - fRadiusX, fCenterY + fRadiusY, fCenterZ - fRadiusZ, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(_pPlane[i], XMVectorSet(fCenterX - fRadiusX, fCenterY - fRadiusY, fCenterZ + fRadiusZ, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(_pPlane[i], XMVectorSet(fCenterX + fRadiusX, fCenterY + fRadiusY, fCenterZ - fRadiusZ, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(_pPlane[i], XMVectorSet(fCenterX + fRadiusX, fCenterY - fRadiusY, fCenterZ + fRadiusZ, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(_pPlane[i], XMVectorSet(fCenterX - fRadiusX, fCenterY + fRadiusY, fCenterZ + fRadiusZ, 1.0f))) >= 0.0f)
            continue;
        if (XMVectorGetX(XMPlaneDotCoord(_pPlane[i], XMVectorSet(fCenterX + fRadiusX, fCenterY + fRadiusY, fCenterZ + fRadiusZ, 1.0f))) >= 0.0f)
            continue;

        return AK_FALSE;
    }

    return AK_TRUE;
}

AkBool FrustumCulling::CheckSphere(AkF32 fCenterX, AkF32 fCenterY, AkF32 fCenterZ, AkF32 fRadius)
{
    // TODO:
    return AK_TRUE;
}
