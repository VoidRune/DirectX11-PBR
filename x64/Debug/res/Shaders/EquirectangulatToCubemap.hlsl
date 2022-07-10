struct VertexInput
{
    float3 position : Position;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 worldPos : Position;
};

cbuffer Cbuff : register(b0)
{
    matrix m;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;

    output.worldPos = input.position;
    output.position = mul(float4(input.position, 1.0), m);
    output.position.z = output.position.w;

    return output;
}

//TextureCube tex : register(t0);
Texture2D tex : register(t0);
SamplerState splr : register(s0);

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
    float2 uv = SampleSphericalMap(normalize(input.worldPos)); // make sure to normalize localPos
    float3 color = tex.Sample(splr, uv).rgb;

    //return tex.Sample(splr, input.worldPos);
    return float4(color, 1.0f);
}