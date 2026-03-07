Texture2D gTextures[1000] : register(t0);
SamplerState gSampler : register(s0);

struct VSInput
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 col : COLOR;
    float texIdx : TEXCOORD1;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 col : COLOR;
    float texIdx : TEXCOORD1;
};
cbuffer UIConstants : register(b0)
{
    float2 screenSize;
    float2 padding;
};

PSInput VS(VSInput input)
{
    PSInput output;
    float2 ndc;
    ndc.x = (input.pos.x / screenSize.x) * 2.0f - 1.0f;
    ndc.y = -((input.pos.y / screenSize.y) * 2.0f - 1.0f);
    output.pos = float4(ndc, 0, 1);
    output.uv = input.uv;
    output.col = input.col;
    output.texIdx = input.texIdx;
    return output;
}

float4 PS(PSInput input) : SV_TARGET
{
    // C'EST CETTE LIGNE QUI FAIT CRASHER LA COMPILATION :
    // On doit forcer l'index en uint et utiliser NonUniformResourceIndex
    uint index = (uint) input.texIdx;
    float4 texColor = gTextures[NonUniformResourceIndex(index)].Sample(gSampler, input.uv);
    
    return texColor * input.col;
}