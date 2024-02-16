#version 450

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal; 

layout(location=0) out vec3 Normal;
layout(location=1) out vec3 FragPos;

//push constants block
layout(push_constant) uniform constants
{
    mat4 model;
    mat4 view;
    mat4 proj;
} PushConstants;

void main() {
    Normal = normalize(mat3(PushConstants.view * PushConstants.model) * aNormal);

    gl_Position = PushConstants.proj *
    PushConstants.view *
    PushConstants.model *
    vec4(aPos.x, aPos.y, aPos.z, 1);

    FragPos = vec3(PushConstants.view * PushConstants.model * vec4(aPos.x, aPos.y, aPos.z, 1.0));
}