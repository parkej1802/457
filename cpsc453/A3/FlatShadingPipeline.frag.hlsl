#include "../ColorUtils.hlsl"

struct PSIn {
    float4 position : SV_POSITION;
    float2 baseColorUV : TEXCOORD0;
    float3 worldNormal : NORMAL;
    float3 viewDir : TEXCOORD1;
};

struct PSOut {
    float4 color : SV_Target0;
};

struct Material
{
    float4 color;
    int hasBaseColorTexture;
    int placeholder0;
    int placeholder1;
    int placeholder2;
};

struct PushConstants 
{
    float4x4 model;
    int perlinNoiseInt;
    int m;
    int AOTextureInt;
    int textureInt;

};

[[vk::push_constant]]
cbuffer {
    PushConstants pushConstants;
};

sampler textureSampler : register(s1, space0);

ConstantBuffer <Material> material: register(b0, space1);

Texture2D baseColorTexture : register(t3, space1);
Texture2D AOColorTexture : register(t2, space1);
Texture2D textureColorTexture : register(t1, space1);

PSOut main(PSIn input) {
    PSOut output;

    float3 color;
    float alpha;
    float PI = 3.14159;
    int m = 24;
    float3 colorPerlin;

    if (pushConstants.textureInt == 1) {
        color = textureColorTexture.Sample(textureSampler, input.baseColorUV);
    }
    else {
        color = material.color.rgb;
        alpha = material.color.a;
    }

    if (pushConstants.perlinNoiseInt == 1) {
        for (int i = 0; i <= 4; i++) {
            colorPerlin += (baseColorTexture.Sample(textureSampler, float2(input.baseColorUV * pow(2, i)))/ pow(2,i));

        }
        colorPerlin = 0.5f * (1.0f + sin(pushConstants.m * PI * (input.baseColorUV.x + input.baseColorUV.y + colorPerlin)));

        color *= colorPerlin;
    }

    if (pushConstants.AOTextureInt == 1) {
        color *= AOColorTexture.Sample(textureSampler, input.baseColorUV);
    }
    
    alpha = 1.0; 

    float3 lightDir = float3(1.0f, 1.0f, 1.0f);
    float ambient = 0.25f * color;
    float specular_power = 76.8;
    float3 specular_strength = float3(1.0, 1, 1);	

    float3 N = normalize(input.worldNormal);
    float3 V = normalize(input.viewDir);
    float3 L = normalize(lightDir);
    float3 R = reflect(-L, N);

    
    float dotProduct = dot(-L, N);
    //diffuse
    float3 dirLight = max(dotProduct, 0.0f) * color;

    float3 specular = pow(max(dot(R, V), 0.0), specular_power) * specular_strength;

    float3 color2 = ambient * color + dirLight + specular ;
    color2 = ApplyExposureToneMapping(color2); 
    color2 = ApplyGammaCorrection(color2); 

    output.color = float4(color2, alpha);
    return output;
}
