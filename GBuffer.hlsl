// Global variab to store a combined view and projection transformation matrix.


matrix WorldViewProj;

texture Tex;

sampler S0 = sampler_state
{
    Texture = (Tex);
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

float4 Blue = { .0f, .0f, 1.0f, 1.0f };

struct VS_INPUT
{
    float4 position : POSITION0;
    float4 normal : NORMAL0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float4 diffuse : COLOR;
};

struct PS_OUTPUT
{
    float4 color : COLOR0;
    float4 n : COLOR1;
};

VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.position = mul(input.position, WorldViewProj);

    output.diffuse = Blue;

    return output;
};

PS_OUTPUT PS_Main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.color = Blue;

    output.n = Blue;

    return output;
}

Technique gbuffer
{
    Pass P0
    {
        VertexShader = compile vs_3_0 VS_Main();
        PixelShader = compile ps_3_0 PS_Main();
    }
}