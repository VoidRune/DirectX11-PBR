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
    float3 reflectedVector : COLOR1;
    float3 refractedVector : COLOR2;

};

cbuffer Cbuff : register(b0)
{
    matrix projection;
    matrix view;
};

cbuffer Cbuff : register(b2)
{
    matrix model;
    float4 camPos;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output;

    float3 viewVector = normalize(mul(float4(input.position, 1), model) - camPos);
    output.position = float4(mul(float4(input.position, 1), mul(mul(model, view), projection)));
    output.normal = mul(input.normal, model);
    output.uv = input.uv;

    output.reflectedVector = reflect(viewVector, output.normal);
    output.refractedVector = refract(viewVector, output.normal, 1.0f / 1.33f);


    return output;
}

TextureCube tex : register(t0);
SamplerState splr : register(s0);

float4 PSMain(PixelInput input) : SV_TARGET
{
    float3 color1 = tex.Sample(splr, input.reflectedVector);
    float3 color2 = tex.Sample(splr, input.refractedVector);

    return float4(color1, 1);

}