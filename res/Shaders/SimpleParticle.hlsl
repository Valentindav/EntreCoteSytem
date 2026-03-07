// SimpleParticle.hlsl — Compute shader de simulation

struct SimpleParticle
{
    float3 position;
    float  life;
    float3 velocity;
    float  maxLife;
    float2 pad;
};

cbuffer ParticleParams : register(b0)
{
    float    gDeltaTime;
    float    gGravity;
    float    gOriginX, gOriginY, gOriginZ;
    float    gSpawnRadius;
    float    gSpeed;
    float    gSpread;
    float    gMinLife;
    float    gMaxLife;
    float    gSizeStart;
    float    gSizeEnd;
    float    gColorStartR, gColorStartG, gColorStartB, gColorStartA;
    float    gColorEndR,   gColorEndG,   gColorEndB,   gColorEndA;
    uint     gMaxParticles;
    float    gEmitRate;
    float    gDeltaAccum;
    float    gPad;
};

RWStructuredBuffer<SimpleParticle> gParticles : register(u0);

float Hash(uint n)
{
    n = (n << 13u) ^ n;
    n = n * (n * n * 15731u + 789221u) + 1376312589u;
    return float(n & 0x7fffffffu) / float(0x7fffffff);
}

float3 RandomDir(uint seed, float spread)
{
    float phi   = Hash(seed * 7u + 1u) * 6.2831853f;
    float cosT  = lerp(1.0f, Hash(seed * 13u + 3u) * 2.0f - 1.0f, spread);
    float sinT  = sqrt(max(0.0f, 1.0f - cosT * cosT));
    return float3(sinT * cos(phi), cosT, sinT * sin(phi));
}

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint idx = id.x;
    if (idx >= gMaxParticles) return;

    SimpleParticle p = gParticles[idx];

    if (p.life > 0.0f)
    {
        p.life       -= gDeltaTime;
        p.velocity.y -= gGravity * gDeltaTime;
        p.position   += p.velocity * gDeltaTime;
        if (p.life < 0.0f) p.life = 0.0f;
    }
    else
    {
        // Spawn aleatoire base sur le taux d'emission
        float chance = gEmitRate * gDeltaTime / float(gMaxParticles);
        uint  seed   = idx * 1337u + uint(gDeltaAccum * 1000.0f);
        if (Hash(seed) < chance)
        {
            float3 origin = float3(gOriginX, gOriginY, gOriginZ);
            // Offset dans un sphere de rayon spawnRadius
            float3 offset = RandomDir(seed + 99u, 1.0f) * (Hash(seed + 77u) * gSpawnRadius);
            float3 dir    = RandomDir(seed, gSpread);

            p.position = origin + offset;
            p.velocity = dir * gSpeed;
            p.maxLife  = lerp(gMinLife, gMaxLife, Hash(seed + 555u));
            p.life     = p.maxLife;
        }
    }

    gParticles[idx] = p;
}
