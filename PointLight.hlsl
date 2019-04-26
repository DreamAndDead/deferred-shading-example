texture diffuseTex;

sampler diffuseSampler = sampler_state
{
    Texture = (diffuseTex);
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = None;
    AddressU = clamp;
    AddressV = clamp;
};



struct VS_INPUT
{
    float4 position : POSITION0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 color : COLOR0;
};

VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = input.position;
    output.texCoord = input.position.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

    return output;
};

PS_OUTPUT PS_Main(VS_OUTPUT input)
{
    PS_OUTPUT output;

    output.color = tex2D(diffuseSampler, input.texCoord);

    return output;
}

Technique main
{
    Pass FrontFace
    {
        VertexShader = compile vs_3_0 VS_Main();
        PixelShader = compile ps_3_0 PS_Main();

        ZWriteEnable = 0;
        ZFunc = ALWAYS;
    }
}
