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
    float3 dummy0;
    float dummy1;
    float3 dummy2;
    float dummy3;
    float3 dummy4;
    float dummy5;

    LightConsts lights[MAX_LIGHTS_COUNT];
};
