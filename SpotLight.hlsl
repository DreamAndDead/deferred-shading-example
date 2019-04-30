// spot light
matrix view;

float2 screenSize;
float viewAspect;
float tanHalfFov;

float3 light_ambient = { 0.2f, 0.2f, 0.2f };
float3 light_diffuse = { 1.f, 1.f, 1.f };
float3 light_specular = { 1.f, 1.f, 1.f };

float3 light_position = { 0.f, 0.f, 5.f };
float3 light_direction = { 0.f, 0.f, -1.f };

float light_range = 8.0f;
float light_falloff = 2.f;

float light_attenuation0 = 0.0f;
float light_attenuation1 = 0.0f;
float light_attenuation2 = 0.1f;

float light_theta = 1.f;
float light_phi = 2.0f;

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

float spot_factor(float3 point_to_light_dir)
{
    float4 light_dir = mul(float4(light_direction, 0), view);
    float3 norm_light_dir = normalize(light_dir.xyz);

    float cos_half_theta = cos(light_theta / 2);
    float cos_half_phi = cos(light_phi / 2);

    // alpha is the angle between light direction vector and light-to-object vector
    float cos_alpha = dot(norm_light_dir, -point_to_light_dir);

    float factor;

    if (cos_alpha > cos_half_theta)
    {
        factor = 1;
    }
    else if (cos_alpha < cos_half_phi)
    {
        factor = 0;
    }
    else
    {
        float p = (cos_alpha - cos_half_phi) / (cos_half_theta - cos_half_phi);
        // p is always between 0 and 1, but hlsl compiler doesn't know
        // use abs() here to avoid the hlsl compiler's warning
        factor = pow(abs(p), light_falloff);
    }

    return factor;
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

	n = normalize(normal);
	v = normalize(-position);

	float4 light_pos = mul(float4(light_position, 1), view);
	l = normalize(light_pos.xyz / light_pos.w - position);
	h = normalize(l + v);

	I_diff = saturate(dot(l, n)) * (diffuse.xyz * light_diffuse.xyz);
	I_spec = pow(saturate(dot(h, n)), shininess) * (specular.xyz * light_specular.xyz);

	att = dist_factor(position);
    spot = spot_factor(l);

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

    // [0, 1] => [-1, 1]
    normal = float4((normal.xyz - 0.5f) * 2, normal.w);
    float d = depth.x * 256.f * 256.f + depth.y * 256.f + depth.z;
    float4 position = float4(input.cameraEye * d, 1);
    float shininess = specular.w * 256.f;

    float3 I_amb = diffuse.rgb * light_ambient;
	float3 I_tot = I_amb + lighting(diffuse.rgb, normal.xyz, position.xyz, specular.rgb, shininess);

    // TODO: how to accumulate multi light shading

	output.color = float4(I_tot, 1);
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
        VertexShader = compile vs_3_0 VS_Main();
        PixelShader = compile ps_3_0 PS_Main();
    }
}
