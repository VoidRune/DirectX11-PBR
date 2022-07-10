StructuredBuffer<float>   inputArray  : register(t0);
RWStructuredBuffer<float> outputArray : register(u0);

[numthreads(1024, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    //int index = id.x + 64 * id.y;
    outputArray[id.x] = inputArray[id.x] + id.x;

}