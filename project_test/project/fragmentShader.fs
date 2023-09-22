#version 330 core

in vec3 color;
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform bool isAuto;
uniform vec4 ncolor;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

float ambientStrength = 0.2f;
float specularStrength = 0.5f;

void main(){
	// FragColor = isAuto ? vec4(color + 0.5f, 1.0f) : ncolor ;

	vec3 normal = normalize(Normal);

	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);

	vec3 ambient = ambientStrength * lightColor;
	vec3 diffuse = max(dot(normal,lightDir),0.0f) * lightColor;
	vec3 specular = pow(max(dot(reflectDir,viewDir),0.0f),32) * specularStrength  * lightColor;
	vec3 result = (ambient + diffuse + specular) * (isAuto ? color : ncolor.xyz);
	FragColor =	vec4(result, 1.0f);
}
