#ifndef __GLOBAL_CONSTS_INCLUDED__
#define __GLOBAL_CONSTS_INCLUDED__

#define MAX_LIGHTS_COUNT 3

struct LightConsts
{
    float3 radiance; // Strength
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
    uint type;
    float radius;
    float dummy0;
    float dummy1;
    Matrix viewProj[5];
};

cbuffer GlobalConsts : register(b0)
{
    matrix view;
    matrix proj;
    float3 eyeWorld;
    float strengthIBL;
    matrix invView;
    matrix invProj;
    
    LightConsts lights[MAX_LIGHTS_COUNT];
};

#endif