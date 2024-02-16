#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec3 Normal;  
layout(location = 1) in vec3 FragPos;

layout( push_constant ) uniform constants 
{
	mat4 model;
	mat4 view;
	mat4 proj;
}PushConstants;

void main() {
	vec3 lightPos = vec3(10, 100, 0);
	vec3 norm = Normal;
    vec3 lightColor = vec3(1.0,1.0,1.0);
    vec3 objectColor = vec3(1.0,1.0,1.0);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

	vec3 cameraPos = vec3(0,0,0);
	float specularStrength = 1;

	vec3 viewDir = normalize(cameraPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    color = vec4(result, 1.0);
}
