#include "proj.h"
#include "Geometry.hpp"
#include "Shader.hpp"

#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

#pragma once

class GUI {
private:
	GLFWwindow* window = nullptr;
	Shader* shader = nullptr;
public:
	GUI(GLFWwindow* window, Shader* shader = nullptr) :window(window),shader(shader) {
		if (shader == nullptr)
			shader = DefaultShader::getDefaultShader();
	}
};
