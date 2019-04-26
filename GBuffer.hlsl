matrix WorldViewProj;

float4 Blue = { .0f, .0f, 1.0f, 1.0f };

struct VS_INPUT
{
    float4 position : POSITION0;
    float4 normal : NORMAL0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
};

struct PS_OUTPUT
{
    float4 normal : COLOR0;
    float4 depth : COLOR1;
    float4 diffuse : COLOR2;
    float4 specular : COLOR3;
};

VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.position = mul(input.position, WorldViewProj);

    return output;
};

PS_OUTPUT PS_Main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.normal = Blue;
    output.depth = Blue;
    output.diffuse = Blue;
    output.specular = Blue;

    return output;
}

Technique main
{
    Pass P0
    {
        VertexShader = compile vs_3_0 VS_Main();
        PixelShader = compile ps_3_0 PS_Main();
    }
}