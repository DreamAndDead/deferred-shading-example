// directional light

matrix view;

float2 screenSize;
float viewAspect;
float tanHalfFov;

float3 light_ambient = { 1.f, 1.f, 1.f };
float3 light_diffuse = { 1.f, 1.f, 1.f };
float3 light_specular = { 1.f, 1.f, 1.f };

float3 light_direction = { 0.f, 0.f, -1.f };

texture normalTex;
sampler normalSampler = sampler_state
{
    Texture = (normalTex);
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = None;
    AddressU = clamp;
    AddressV = clamp;
};

texture depthTex;
sampler depthSampler = sampler_state
{
    Texture = (depthTex);
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = None;
    AddressU = clamp;
    AddressV = clamp;
};

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

texture specularTex;
sampler specularSampler = sampler_state
{
    Texture = (specularTex);
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
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 cameraEye : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 color : COLOR0;
};

/*
 * diffuse: object material diffuse color
 * normal: object normal vector in camera space
 * position: object position in camera space
 * specular: object material specular
 */
float3 lighting(float3 diffuse, float3 normal, float3 position, float3 specular, float shininess)
{
    float3 I_diff, I_spec, I_total;
    float3 l, v, n, h;
    float att;

    n = normalize(normal);
    v = normalize(-position);

    att = 1;

    // tranform light direction from wolrd space to camera space
    float4 light_dir = mul(float4(light_direction, 0), view);
    l = normalize(-light_dir.xyz);

    I_diff = saturate(dot(l, n)) * (diffuse.xyz * light_diffuse.xyz);

    h = normalize(l + v);

    I_spec = saturate(dot(l, n)) * pow(saturate(dot(h, n)), shininess) * (specular.xyz * light_specular.xyz);

    I_total = att * (I_diff + I_spec);
    return I_total;
}

VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = input.position;
    output.texCoord = input.position.xy * float2(0.5, -0.5) + float2(0.5, 0.5) + 0.5 / screenSize;

    // field of view: in y direction
    output.cameraEye = float3(input.position.x * tanHalfFov * viewAspect, input.position.y * tanHalfFov, 1);

    return output;
};

PS_OUTPUT PS_Main(VS_OUTPUT input)
{
    PS_OUTPUT output;

    float4 normal = tex2D(normalSampler, input.texCoord);
    float4 depth = tex2D(depthSampler, input.texCoord);
    float4 diffuse = tex2D(diffuseSampler, input.texCoord);
    float4 specular = tex2D(specularSampler, input.texCoord);

    normal = float4((normal.xyz - 0.5f) * 2, normal.w);
    float4 position = float4(input.cameraEye * depth.x * 100, 1);
    float shininess = specular.w;

	float3 total_color = diffuse.rgb;

	total_color = total_color + lighting(diffuse.rgb, normal.xyz, position.xyz, specular.rgb, shininess);

    // don't work with the alpha
	output.color = float4(total_color, 1);
    return output;
}

Technique Plain
{
    Pass Light
    {
        VertexShader = compile vs_3_0 VS_Main();
        PixelShader = compile ps_3_0 PS_Main();
    }
}

// no Technique StencilCulling for directional light
