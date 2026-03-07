// ==============================================================================
// DEFINES & CONSTANTS
// ==============================================================================
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define MAX_LIGHTS 16

// ==============================================================================
// STRUCTURES DE DONNÉES
// ==============================================================================
struct LightData
{
    int Type; // 0: Directional, 1: Point, 2: Spot
    float3 Color;
    
    float3 Position;
    float Range;
    
    float3 Direction;
    float SpotInnerAngle;
    
    float3 Attenuation;
    float SpotOuterAngle;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    
    float3 BoneWeights : BLENDWEIGHT;
    uint4 BoneIndices : BLENDINDICES;
};

struct InstanceIn
{
    float4 Color : COLOR;
    float4 World0 : WORLD0;
    float4 World1 : WORLD1;
    float4 World2 : WORLD2;
    float4 World3 : WORLD3;
    uint TexIndex : TEXINDEX;
    uint MatIndex : MATINDEX;
    
    uint BoneOffset : BONEOFFSET;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float4 Color : COLOR;
    float2 TexC : TEXCOORD;
    uint TexIndex : TEXINDEX;
    uint MatIndex : MATINDEX;
};

// ==============================================================================
// BUFFERS & RESSOURCES
// ==============================================================================
cbuffer cbPerObject : register(b0)
{
    row_major float4x4 ViewProj;
    float3 EyePosW;
    float padding;
};

cbuffer cbLights : register(b1)
{
    LightData gLights[MAX_LIGHTS];
    int gLightCount;
    float3 gPad;
};

Texture2D gTextures[100] : register(t0);
StructuredBuffer<float4x4> gBoneTransforms : register(t100);

SamplerState gsamPoint : register(s0);

// ==============================================================================
// FONCTIONS UTILITAIRES
// ==============================================================================
float3 ComputeLight(LightData light, float3 posW, float3 normalW)
{
    float3 lightDir = float3(0.0f, 0.0f, 0.0f);
    float attenuation = 1.0f;

    if (light.Type == LIGHT_DIRECTIONAL)
    {
        lightDir = normalize(-light.Direction);
    }
    else if (light.Type == LIGHT_POINT || light.Type == LIGHT_SPOT)
    {
        float3 lightVec = light.Position - posW;
        float distance = length(lightVec);
        
        if (distance > light.Range)
            return float3(0.0f, 0.0f, 0.0f);
        
        lightDir = lightVec / distance;
        attenuation = 1.0f / dot(light.Attenuation, float3(1.0f, distance, distance * distance));

        if (light.Type == LIGHT_SPOT)
        {
            float cosAngle = dot(-lightDir, normalize(light.Direction));
            float spotIntensity = smoothstep(cos(light.SpotOuterAngle), cos(light.SpotInnerAngle), cosAngle);
            attenuation *= spotIntensity;
        }
    }
    else
    {
        return float3(0.0f, 0.0f, 0.0f); // Type inconnu, on ne fait rien
    }

    float nDotL = saturate(dot(normalW, lightDir));
    return light.Color * nDotL * attenuation;
}

// ==============================================================================
// VERTEX SHADER
// ==============================================================================
VertexOut VS(VertexIn vin, InstanceIn iin)
{
    VertexOut vout;

    float4x4 World = float4x4(iin.World0, iin.World1, iin.World2, iin.World3);
    float4x4 boneTransform;

    // 0xFFFFFFFF (4294967295) indique que cette instance n'a pas de Bones
    if (iin.BoneOffset == 0xFFFFFFFF)
    {
        // On force la matrice Identité (pas de déformation)
        boneTransform = float4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }
    else
    {
        // On calcule le skinning normalement
        float weight4 = 1.0f - (vin.BoneWeights.x + vin.BoneWeights.y + vin.BoneWeights.z);
        
        boneTransform =
            gBoneTransforms[iin.BoneOffset + vin.BoneIndices.x] * vin.BoneWeights.x +
            gBoneTransforms[iin.BoneOffset + vin.BoneIndices.y] * vin.BoneWeights.y +
            gBoneTransforms[iin.BoneOffset + vin.BoneIndices.z] * vin.BoneWeights.z +
            gBoneTransforms[iin.BoneOffset + vin.BoneIndices.w] * weight4;
    }
    
    float4x4 finalTransform = mul(boneTransform, World);
    
    float4 posW = mul(float4(vin.PosL, 1.0f), finalTransform);
    vout.PosW = posW.xyz;

    vout.NormalW = mul(vin.NormalL, (float3x3) finalTransform);
    vout.NormalW = normalize(vout.NormalW);

    vout.PosH = mul(posW, ViewProj);

    vout.Color = iin.Color;
    vout.TexC = vin.TexC;
    vout.TexIndex = iin.TexIndex;
    vout.MatIndex = iin.MatIndex;

    return vout;
}

// ==============================================================================
// PIXEL SHADERS
// ==============================================================================
float4 PS_Solid(VertexOut pin) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);

    float4 texColor = gTextures[pin.TexIndex].Sample(gsamPoint, pin.TexC);
    float4 baseColor = texColor * pin.Color;

    float3 totalLight = float3(0.0f, 0.0f, 0.0f);
    float3 ambientLight = float3(0.1f, 0.1f, 0.1f) * baseColor.rgb;

    for (int i = 0; i < gLightCount; ++i)
    {
        totalLight += ComputeLight(gLights[i], pin.PosW, pin.NormalW);
    }

    float3 finalColor = ambientLight + (baseColor.rgb * totalLight);

    return float4(finalColor, baseColor.a);
}

float4 PS_Wireframe(VertexOut pin) : SV_Target
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f);
}