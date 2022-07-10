struct VertexInput
{
    float3 position : Position;
    float3 normal : Normal;
    float2 uv : TexCoord;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 normal : Normal;
    float2 uv : TexCoord;
    float3 worldPos : COLOR0;
    float3 cameraPos : COLOR1;

};

cbuffer Camera : register(b0)
{
    matrix projection;
    matrix view;
};

cbuffer Model : register(b2)
{
    matrix model;
    float4 camPos;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;

    output.worldPos = mul(float4(input.position, 1), model);
    output.position = mul(float4(output.worldPos, 1), mul(view, projection));
    output.normal = mul(input.normal, model);
    output.uv = input.uv;
    output.cameraPos = camPos;

    return output;
}

TextureCube cubeMap : register(t0);

Texture2D AlbedoTex : register(t1);
Texture2D NormalTex : register(t2);
Texture2D MetallicTex : register(t3);
Texture2D RoughnessTex : register(t4);
Texture2D AO : register(t5);

TextureCube irradianceMap : register(t6);
TextureCube prefilteredMap : register(t7);
Texture2D BDRF : register(t8);

SamplerState splr : register(s0);

#define PI 3.14159265359

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float2 SampleSphericalMap(float3 v)
{
    float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float4 PSMain(PixelInput input) : SV_TARGET
{
    //float3 color1 = tex.Sample(splr, reflect(input.worldPos - input.cameraPos, input.normal));
    //float3 color2 = tex.Sample(splr, refract(input.worldPos - input.cameraPos, input.normal, 1.0f / 1.33f));

    float3 albedo = AlbedoTex.Sample(splr, input.uv);
    //float3 albedo = float3(1.0f, 0.6172f, 0.1384f); //GOLD

    float metallic = MetallicTex.Sample(splr, input.uv).r;
    float roughness = RoughnessTex.Sample(splr, input.uv).r;
    float ao = AO.Sample(splr, input.uv).r;

    float3 tangentNormal = NormalTex.Sample(splr, input.uv).rgb;

    float3 Q1 = ddx(input.worldPos);
    float3 Q2 = ddy(input.worldPos);
    float3 st1 = ddx(float3(input.uv, 0));
    float3 st2 = ddy(float3(input.uv, 0));

    float3 Na = normalize(input.normal);
    float3 Ta = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 Ba = -normalize(cross(Na, Ta));
    float3x3 TBN = float3x3(Ta, Ba, Na);

    float3 normal = normalize(mul(tangentNormal, TBN));

    float3 N = normalize(input.normal);
    float3 V = normalize(input.cameraPos - input.worldPos);
    float3 R = reflect(-V, N);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    // reflectance equation
    //float3 Lo = float3(0.0, 0.0, 0.0);

    //float3 lightPositions[4];
    //lightPositions[0] = float3(-1, -1, -1);
    //lightPositions[1] = float3(-1, -1,  1);
    //lightPositions[2] = float3( 1,  1,  1);
    //lightPositions[3] = float3( 1,  1, -1);
    //float3 lightColours[4];
    //lightColours[0] = float3(1, 1, 1);
    //lightColours[1] = float3(1, 1, 1);
    //lightColours[2] = float3(1, 1, 1);
    //lightColours[3] = float3(1, 1, 1);

    //for (int i = 0; i < 4; i++)
    //{
    //    // calculate per-light radiance
    //    float3 L = normalize(lightPositions[i] - input.worldPos);
    //    float3 H = normalize(V + L);
    //    float distance = length(lightPositions[i] - input.worldPos);
    //    float attenuation = 1.0 / (distance * distance);
    //    //float attenuation = 10.0 / (distance);
    //    float3 radiance = lightColours[i] * attenuation;
    //
    //    // cook-torrance brdf
    //    float NDF = DistributionGGX(N, H, roughness);
    //    float G = GeometrySmith(N, V, L, roughness);
    //    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    //
    //    float3 kS = F;
    //    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    //    kD *= 1.0 - metallic;
    //
    //    float3 numerator = NDF * G * F;
    //    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    //    float3 specular = numerator / denominator;
    //
    //    // add to outgoing radiance Lo
    //    float NdotL = max(dot(N, L), 0.0);
    //    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    //}

    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    float3 irradiance = irradianceMap.Sample(splr, N).rgb;
    float3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = prefilteredMap.SampleLevel(splr, R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 envBRDF = BDRF.Sample(splr, float2(max(dot(N, V), 0.0), roughness)).rg;
    // Environment Bidirectional reflectance distribution function
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    float3 ambient = (kD * diffuse + specular) * ao;

    //float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    float3 color = ambient;// +Lo;

    return float4(color, 1.0f);

}