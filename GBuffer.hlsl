matrix world;
matrix view;
matrix proj;

float3 blue = { .0f, .0f, 1.0f};
float3 white = { 1.0f, 1.0f, 1.0f};

float shininess = 5.0f;

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
    float4 viewPos = mul(input.position, worldView);
    float4 projPos = mul(viewPos, proj);

    output.position = projPos;

    float4 n = mul(float4(input.normal.xyz, 0), worldView);
    n = normalize(n);
    // store normal from range [-1, 1] => color range [0, 1]
    output.normal = float4(n.xyz / 2 + 0.5f, 0);

    // depth in view coordinate, store in x
    float depth = (float) viewPos.z / (float) viewPos.w;
    output.depth = float2(depth, 0);

    return output;
};

PS_OUTPUT PS_Main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;

    output.normal = float4(input.normal.xyz, 0);

    // zip depth in color range [0, 1]
    float depth = input.depth.x;
    float int_depth = floor(depth);
    float dec_depth = frac(depth);
    output.depth = float4((int_depth / 256.f) / 256.f, (int_depth % 256.f) / 256.f, dec_depth, 0);

    output.diffuse = float4(blue, 0);
    output.specular = float4(white, shininess / 256.f);

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