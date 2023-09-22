#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
out vec3 color;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 modelBuffer;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	// gl_Position = view * model * modelBuffer * vec4(aPos, 1.0f);
	gl_Position = projection * view * model * modelBuffer * vec4(aPos, 1.0f);
	color = aPos;
	Normal = mat3(transpose(inverse(model*modelBuffer))) * aNormal;
	FragPos = vec3(model * modelBuffer * vec4(aPos, 1.0f));
}
