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
	Shader() {
		ID = glCreateProgram();
	}
	Shader(std::string vertexPath, std::string fragmentPath) {
		ID = glCreateProgram();
		compileAndAttachShader(vertexPath, GL_VERTEX_SHADER);
		compileAndAttachShader(fragmentPath, GL_FRAGMENT_SHADER);
		link();
	}
	Shader(std::string vertexPath, std::string geometryPath, std::string fragmentPath) {
		ID = glCreateProgram();
		compileAndAttachShader(vertexPath, GL_VERTEX_SHADER);
		compileAndAttachShader(geometryPath, GL_GEOMETRY_SHADER);
		compileAndAttachShader(fragmentPath, GL_FRAGMENT_SHADER);
		link();
	}
	~Shader() {
		glDeleteProgram(ID);
	}
	GLuint getID() {
		GLint success;
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (success != GL_TRUE) {
			std::cout << "返回未链接成功的shader！" << std::endl;	// 调试输出
		}
		return ID;
	}
	GLint compileAndAttachShader(std::string shaderSourcePath, GLenum shaderType) {
		GLuint shader = glCreateShader(shaderType);
		loadShader(shader, readSource(shaderSourcePath));

		GLint success;
		GLchar infoLog[512];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			glGetShaderInfoLog(shader, length * sizeof(GLchar), NULL, infoLog);
			std::cout << "着色器 " << shaderType << " 编译失败！\n" << infoLog << std::endl;			// 调试输出
		}

		glAttachShader(ID, shader);
		return GL_TRUE;
	}
	GLint link() {
		glLinkProgram(ID);
		GLint success;
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (success != GL_TRUE) {
			char info[512];
			GLsizei length;
			glGetProgramInfoLog(ID, 512, &length, info);
			std::cout << "program链接失败！\n" << info << std::endl;
			exit(5);
		}
		return success;
	}
	void use() {
		GLint success;
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (success == GL_TRUE)
			glUseProgram(ID);
		else {
			std::cout << "使用未正确链接的着色器program！" << std::endl;
			exit(6);
		}
	}
	ShaderUniform operator[](const std::string& name) {
		ShaderUniform su(glGetUniformLocation(ID, name.c_str()));
		return su;
	}
};


