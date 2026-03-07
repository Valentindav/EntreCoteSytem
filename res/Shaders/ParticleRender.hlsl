#pragma pack_matrix(row_major)

struct SimpleParticle
{
    float3 position;
    float  life;
    float3 velocity;
    float  maxLife;
    float2 pad;
};

// PassConstants C++ layout:
//   XMFLOAT4X4 ViewProj  (64 bytes, offset 0)
//   XMFLOAT4X4 View      (64 bytes, offset 64)
//   XMFLOAT3   EyePosW   (12 bytes, offset 128)
//   float      TotalTime  (4 bytes, offset 140)
cbuffer PassCB : register(b0)
{
    float4x4 gViewProj;
    float4x4 gView;
    float3   gEyePosW;
    float    gTotalTime;
};

cbuffer ParticleRenderCB : register(b1)
{
    float  gSizeStart;
    float  gSizeEnd;
    float  gPad0;
    float  gPad1;
    float4 gColorStart;
    float4 gColorEnd;
};

StructuredBuffer<SimpleParticle> gParticles : register(t0);

struct VSOut
{
    float4 posH  : SV_POSITION;
    float4 color : COLOR0;
    float2 uv    : TEXCOORD0;
};

static const float2 Corners[6] =
{
    float2(-1,  1), float2( 1,  1), float2(-1, -1),
    float2(-1, -1), float2( 1,  1), float2( 1, -1),
};
static const float2 UVs[6] =
{
    float2(0, 0), float2(1, 0), float2(0, 1),
    float2(0, 1), float2(1, 0), float2(1, 1),
};

VSOut VS(uint vid : SV_VertexID)
{
    VSOut o;
    uint pIdx   = vid / 6u;
    uint corner = vid % 6u;

    SimpleParticle p = gParticles[pIdx];

    if (p.life <= 0.0f || p.maxLife <= 0.0f)
    {
        o.posH  = float4(0, 0, -10, 1);
        o.color = float4(0, 0, 0, 0);
        o.uv    = float2(0, 0);
        return o;
    }

    float t    = 1.0f - saturate(p.life / p.maxLife);
    float size = lerp(gSizeStart, gSizeEnd, t);

    float3 right = normalize(float3(gView[0][0], gView[1][0], gView[2][0]));
    float3 up = normalize(float3(gView[0][1], gView[1][1], gView[2][1]));

    float3 wPos = p.position
                + right * Corners[corner].x * size
                + up * Corners[corner].y * size;

    o.posH  = mul(float4(wPos, 1.0f), gViewProj);
    o.color = lerp(gColorStart, gColorEnd, t);
    o.uv    = UVs[corner];
    return o;
}

float4 PS(VSOut i) : SV_Target
{
    float2 uv = i.uv * 2.0f - 1.0f;
    float  d  = dot(uv, uv);
    if (d > 1.0f) discard;
    float4 col = i.color;
    col.a *= 1.0f - smoothstep(0.5f, 1.0f, d);
    if (col.a < 0.01f) discard;
    return col;
}
