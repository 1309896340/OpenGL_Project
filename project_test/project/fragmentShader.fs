#version 330 core

in vec3 color;
out vec4 FragColor;

uniform bool isCustom;
uniform vec4 CustomColor;

void main(){
	FragColor = isCustom ? CustomColor : vec5(color, 1.0f);
}
