#include "proj.h"
#include "utils.h"

#pragma once


class ShaderUniform {
private:
	GLint location;
public:
	ShaderUniform(GLint location) :location(location) {}
	void operator=(glm::vec4 value) {
		glUniform4fv(location, 1, glm::value_ptr(value));
	}
	void operator=(glm::vec3 value) {
		glUniform3fv(location, 1, glm::value_ptr(value));
	}
	void operator=(glm::mat4 value) {
		glUniformMatrix4fv(location, 1, GLFW_FALSE, glm::value_ptr(value));
	}
	void operator=(bool value) {
		glUniform1i(location, (int)value);
	}
};

class Shader {
private:
	GLuint ID;
public:
	Shader(std::string vertexPath, std::string fragmentPath) {
		ID = loadProgramFromFile(vertexPath, fragmentPath);
	}
	void use() {
		glUseProgram(ID);
	}
	void close() {
		glUseProgram(0);
	}
	ShaderUniform operator[](const std::string &name) {
		ShaderUniform su(glGetUniformLocation(ID, name.c_str()));
		return su;
	}
};


