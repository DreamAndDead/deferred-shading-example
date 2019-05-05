// directional light

matrix view;

float2 screenSize;
float viewAspect;
float tanHalfFov;



float3 light_ambient;
float3 light_diffuse;
float3 light_specular;

float3 light_direction;



/* not used light parameters */
float3 light_position;

float light_range;
float light_falloff;

float light_attenuation0;
float light_attenuation1;
float light_attenuation2;

float light_theta;
float light_phi;
/* not used light parameters */

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

texture stashTex;
sampler stashSampler = sampler_state
{
    Texture = (stashTex);
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
 * diffuse: material diffuse color
 * normal: point normal vector in camera space
 * position: point position in camera space
 * specular: material specular
 * shininess: shininess parameter
 */
float3 lighting(float3 diffuse, float3 normal, float3 position, float3 specular, float shininess)
{
    float3 I_diff, I_spec;
    float3 l, v, n, h;
    float att, spot;

    att = 1;
    spot = 1;

    n = normalize(normal);
    v = normalize(-position);

    // tranform light direction from wolrd space to camera space
    float4 light_dir = mul(float4(light_direction, 0), view);
    l = normalize(-light_dir.xyz);
    h = normalize(l + v);

    I_diff = saturate(dot(l, n)) * (diffuse * light_diffuse);
    I_spec = pow(saturate(dot(h, n)), shininess) * (specular * light_specular);

    return (att * spot) * (I_diff + I_spec);
}


VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = input.position;
    output.texCoord = input.position.xy * float2(0.5, -0.5) + float2(0.5, 0.5) + 0.5 / screenSize;
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
    float4 stash = tex2D(stashSampler, input.texCoord);

    // [0, 1] => [-1, 1]
    normal = float4((normal.xyz - 0.5f) * 2, normal.w);
    float d = depth.x * 256.f * 256.f + depth.y * 256.f + depth.z;
    float4 position = float4(input.cameraEye * d, 1);
    float shininess = specular.w * 256.f;

    float3 I_amb = diffuse.rgb * light_ambient;
	float3 I_tot = I_amb + lighting(diffuse.rgb, normal.xyz, position.xyz, specular.rgb, shininess);

	output.color = float4(I_tot + stash.rgb, 1);
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
// just copy Plain Technique for code simplicity
Technique StencilCulling
{
    Pass Light
    {
        VertexShader = compile vs_3_0 VS_Main();
        PixelShader = compile ps_3_0 PS_Main();
    }
}

