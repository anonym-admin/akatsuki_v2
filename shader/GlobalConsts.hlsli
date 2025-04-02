#ifndef __GLOBAL_CONSTS_INCLUDED__
#define __GLOBAL_CONSTS_INCLUDED__

#define MAX_LIGHTS_COUNT 3
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

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
    float haloRadius;
    float haloStrength;
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
    
    float globalTime;
    float3 dummy;
    
    LightConsts lights[MAX_LIGHTS_COUNT];
};

#endif