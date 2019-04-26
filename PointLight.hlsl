float4 Blue = { .0f, .0f, 1.0f, 1.0f };

texture diffuseTex;

sampler S0 = sampler_state
{
    Texture = (diffuseTex);
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
};

struct VS_INPUT
{
    float4 position : POSITION0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
};

struct PS_OUTPUT
{
    float4 color : COLOR0;
};

VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = float4(1, 0, 0, 1);

    return output;
};

PS_OUTPUT PS_Main(VS_OUTPUT input)
{
    PS_OUTPUT output;
    float2 pos = input.position.xy;

    output.color = tex2D(S0, pos);

    return output;
}

Technique main
{
    Pass FrontFace
    {
        VertexShader = compile vs_3_0 VS_Main();
        PixelShader = compile ps_3_0 PS_Main();

        ZWriteEnable = 0;
        ZFunc = LESS;
    }
}