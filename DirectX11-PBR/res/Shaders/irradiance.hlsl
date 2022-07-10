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

#define PI 3.14159265359
float4 PSMain(PixelInput input) : SV_TARGET
{
	float3 normal = normalize(input.worldPos);
	float3 irradiance = float3(0.0, 0.0, 0.0);

	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = cross(up, normal);
	up = cross(normal, right);

	float sampleDelta = 0.012;
	float nrSamples = 0.0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += tex.Sample(splr, SampleSphericalMap(sampleVec)).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0);
}