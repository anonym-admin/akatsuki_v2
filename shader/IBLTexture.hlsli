TextureCube irradianceIBLTex : register(t11);
TextureCube specularIBLTex : register(t12);
Texture2D brdfTex : register(t13);

cbuffer MaterialConsts : register(b2)
{
    float3 albedoFactor;
    float roughnessFactor;
    float3 emissionFactor;
    float metallicFactor;

    uint useAlbedoMap;
    uint useNormalMap;
    uint useEimissiveMap;
    uint useAOMap;
    uint invertNormalMapY;
    uint useMetallicMap;
    uint useRoughnessMap;
    float reserve0;
};