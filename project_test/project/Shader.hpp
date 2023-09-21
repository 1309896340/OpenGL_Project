#include "proj.h"
#include "utils.h"

#pragma once

class Shader {
private:
	GLuint ID;
public:
	Shader(std::string vertexPath,std::string fragmentPath) {
		ID = loadProgramFromFile(vertexPath, fragmentPath);
	}
	void use() {
		glUseProgram(ID);
	}
	void close() {
		glUseProgram(0);
	}
	void setVec4(std::string name, glm::vec4 value) {
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
	}
	void setMat4(std::string name, glm::mat4 value) {
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
	}
	void setBool(std::string name, bool value) {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
};
