struct VSIn {
    float3 position : POSITION0;
    float2 baseColorUV : TEXCOORD0;
    float3 normal : NORMAL;
};

struct VSOut {
    float4 position : SV_POSITION;
    float2 baseColorUV : TEXCOORD0;
    float3 worldNormal : NORMAL;
    float3 viewDir : TEXCOORD1;
};

struct ViewProjectionBuffer {
    float4x4 viewProjection;
};

ConstantBuffer <ViewProjectionBuffer> vpBuff: register(b0, space0);

struct PushConsts
{
    float4x4 model;
};

[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

VSOut main(VSIn input) {
    VSOut output;

    float4 worldPosition = mul(pushConsts.model, float4(input.position, 1.0));
    output.position = mul(vpBuff.viewProjection, worldPosition);
    output.baseColorUV = input.baseColorUV;
    output.worldNormal = normalize(mul((float3x3)pushConsts.model, input.normal));
    //output.viewDir = normalize(float3(0.0f, 0.0f, 0.0f) - worldPosition.xyz);
    float3 cameraPosition = float3(vpBuff.viewProjection._41, vpBuff.viewProjection._42, vpBuff.viewProjection._43);
    output.viewDir = normalize(cameraPosition - worldPosition.xyz);

    return output;
}