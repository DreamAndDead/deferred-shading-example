matrix world;
matrix view;
matrix proj;

float4 C = { .0f, .0f, 1.0f, 1.0f };

struct VS_INPUT
{
    float4 position : POSITION0;
    float4 normal : NORMAL0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float4 normal : NORMAL;
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


    matrix worldView = mul(world, view);
    matrix worldViewProj = mul(mul(world, view), proj);

    float4 viewPos = mul(input.position, worldView);
    float4 projPos = mul(viewPos, proj);

    output.position = projPos / projPos.w;

    output.normal = mul(float4(input.normal.xyz, 0), worldViewProj);

    // store depth in normal.w
    // depth in view coordinate
    output.normal.w = viewPos.z / viewPos.w;

    // maybe from texture
    // output.diffuse
    // output.specular

    return output;
};

PS_OUTPUT PS_Main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.normal = float4(input.normal.xyz, 1);
    output.depth = float4(input.normal.w, 0, 0, 1);
    output.diffuse = C;
    output.specular = C;

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