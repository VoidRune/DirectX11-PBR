struct VertexInput
{
    float2 position : Position;
    float2 uv : TextureCoord;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TextureCoord;
};

Texture2D tex : register(t0);
SamplerState splr : register(s0);

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.uv = input.uv;
    return output;
}

#define INTENSITY 0.4f
float4 PSMain(PixelInput input) : SV_TARGET
{
    uint width, height;
    tex.GetDimensions(width, height);
    float3 color = tex.Sample(splr, input.uv);
    
    //Pixels towards the edge get darker
    float dist = 1.0f - distance(float2((input.position.x - width / 2) / width, (input.position.y - height / 2) / height) * INTENSITY, float2(0.0f, 0.0f));
    color *= dist;
    
    /* Gamma correction */
    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0));


    return float4(color, 1.0f);
}