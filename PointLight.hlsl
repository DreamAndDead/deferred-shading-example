// point light
matrix world;
matrix view;
matrix proj;

float2 screenSize;
float viewAspect;
float tanHalfFov;




float3 light_ambient;
float3 light_diffuse;
float3 light_specular;

float3 light_position;

float light_range;

float light_attenuation0;
float light_attenuation1;
float light_attenuation2;




/* not used light parameters */
float3 light_direction;

float light_falloff;

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
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float3 cameraEye : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 color : COLOR0;
};

float dist_factor(float3 point_view_pos)
{
	float4 light_pos = mul(float4(light_position, 1), view);
	float dist = distance(point_view_pos, light_pos.xyz / light_pos.w);

	float dist_att;
	if (dist > light_range)
	{
		dist_att = 0;
	}
	else
	{
		dist_att = 1 / (light_attenuation0 + light_attenuation1 * dist + light_attenuation2 * dist * dist);
	}

	return dist_att;
}

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

	att = dist_factor(position);
    spot = 1;

	n = normalize(normal);
	v = normalize(-position);

	float4 light_pos = mul(float4(light_position, 1), view);
	l = normalize(light_pos.xyz / light_pos.w - position);
	h = normalize(l + v);

	I_diff = saturate(dot(l, n)) * (diffuse.xyz * light_diffuse.xyz);
	I_spec = pow(saturate(dot(h, n)), shininess) * (specular.xyz * light_specular.xyz);

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




struct VS1_IN
{
    float4 pos : POSITION;
};

struct VS1_OUT
{
    float4 pos : POSITION;
};

struct PS1_OUT
{
    float4 color : COLOR0;
};

VS1_OUT VS1(VS1_IN input)
{
    VS1_OUT output;

    float4 p = input.pos;

    float4x4 worldViewProj = mul(mul(world, view), proj);
    p = mul(p, worldViewProj);

    output.pos = p;

    return output;
}

PS1_OUT PS1(VS1_OUT input)
{
    PS1_OUT output;
    output.color = float4(1, 1, 1, 1);
    return output;
}

VS_OUTPUT VS_BackFace(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 local_pos = input.position;
    float4x4 worldViewProj = mul(mul(world, view), proj);
    float4 proj_pos = mul(local_pos, worldViewProj);

    output.position = proj_pos;

    float4 norm_proj_pos = proj_pos / proj_pos.w;
    output.cameraEye = float3(norm_proj_pos.x * tanHalfFov * viewAspect, norm_proj_pos.y * tanHalfFov, 1);

    // -0.5, because the tex coord and proj screen coord are opposite
    // 0.5 / screenParam
    // ref https://docs.microsoft.com/en-us/windows/desktop/direct3d10/d3d10-graphics-programming-guide-resources-coordinates
    // and https://docs.microsoft.com/en-us/windows/desktop/direct3d9/directly-mapping-texels-to-pixels
    output.texCoord = norm_proj_pos.xy * float2(0.5, -0.5) + float2(0.5, 0.5) + 0.5 / screenSize;

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

Technique StencilCulling
{
    Pass FrontFace
    {
        VertexShader = compile vs_3_0 VS1();
        PixelShader = compile ps_3_0 PS1();

        ColorWriteEnable = 0;
        ZWriteEnable = 0;
        ZFunc = LESS;

        StencilEnable = true;
        StencilFunc = ALWAYS;
        StencilZFail = REPLACE;
        StencilPass = KEEP;
        StencilRef = 1;
        StencilMask = 0xFFFFFFFF;

        CullMode = CCW;
    }
    Pass BackFace
    {
        VertexShader = compile vs_3_0 VS_BackFace();
        PixelShader = compile ps_3_0 PS_Main();

        ColorWriteEnable = 0xFFFFFFFF;
        ZWriteEnable = 0;
        ZFunc = GREATEREQUAL;

        StencilEnable = true;
        StencilFunc = EQUAL;
        StencilPass = KEEP;
        StencilRef = 0;
        StencilMask = 0xFFFFFFFF;

        CullMode = CW;

    }
}
