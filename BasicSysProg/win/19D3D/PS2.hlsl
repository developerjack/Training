
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
    float4 vLightDir;
    float4 vLightColor;
}

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Norm : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target
{
    return saturate(dot(vLightDir,input.Norm) * vLightColor);
}