matrix world;
matrix view;
matrix proj;

float4 Blue = { .0f, .0f, 0.6f, 1.0f };
float4 White = { 1.0f, 1.0f, 1.0f, 1.0f };

struct VS_INPUT
{
    float4 position : POSITION0;
    float4 normal : NORMAL0;
};

struct VS_OUTPUT
{
    float4 position : POSITION0;
    float4 normal : NORMAL0;
    float2 depth : TEXCOORD0;
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
    VS_OUTPUT output;

    matrix worldView = mul(world, view);
    matrix worldViewProj = mul(worldView, proj);

    float4 viewPos = mul(input.position, worldView);
    float4 projPos = mul(input.position, worldViewProj);

    output.position = projPos / projPos.w;

    float4 n = mul(float4(input.normal.xyz, 0), worldView);
    n = normalize(n);
    // ATTENTION: normal range [-1, 1] => color range [0, 1]
    n = float4(n.xyz / 2 + 0.5f, 0);
    output.normal = n;

    // depth in view coordinate
    // store in x
    output.depth = (viewPos.z * 100 / viewPos.w, 0);

    // from texture
    // output.diffuse
    // output.specular

    return output;
};

PS_OUTPUT PS_Main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;

    output.normal = float4(input.normal.xyz, 0);
    output.depth = float4(input.depth.x, 0, 0, 0);
    output.diffuse = Blue;
    output.specular = White;

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