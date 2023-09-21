#version 330 core

in vec3 color;
out vec4 FragColor;

uniform bool isAuto;
uniform vec4 ncolor;

void main(){
	FragColor = isAuto ? vec4(color + 0.5f, 1.0f) : ncolor ;
}
