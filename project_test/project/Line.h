#pragma once

#include "proj.h"
#include "Shader.hpp"

typedef struct {
	vec3 begin;
	vec3 end;
	vec4 color;
	float width;
} Line;

typedef struct {
	std::vector<Line> lines;	// ÏßÌõ»º³åÇø
	GLuint vbo_line;
	Shader* shader;
}LineStructure;

