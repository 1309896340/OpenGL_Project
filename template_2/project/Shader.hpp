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
	Shader() :ID(glCreateProgram()) {}
	Shader(std::string vertexPath, std::string fragmentPath) :ID(glCreateProgram()) {
		compileAndAttachShader(vertexPath, GL_VERTEX_SHADER);
		compileAndAttachShader(fragmentPath, GL_FRAGMENT_SHADER);
		link();
	}
	Shader(std::string vertexPath, std::string geometryPath, std::string fragmentPath) :ID(glCreateProgram()) {
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
			std::cout << "����δ���ӳɹ���shader��" << std::endl;	// �������
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
			std::cout << "��ɫ�� " << shaderType << " ����ʧ�ܣ�\n" << infoLog << std::endl;			// �������
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
			std::cout << "program����ʧ�ܣ�\n" << info << std::endl;
			exit(5);
		}
		return success;
	}
	void use() {		// ��glDrawXXX()ǰ��ؼǵõ��ô˺���
		GLint success;
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (success == GL_TRUE)
			glUseProgram(ID);
		else {
			std::cout << "ʹ��δ��ȷ���ӵ���ɫ��program��" << std::endl;
			exit(6);
		}
	}
	ShaderUniform operator[](const std::string& name) {
		this->use();
		ShaderUniform su(glGetUniformLocation(ID, name.c_str()));
		return su;
	}

};

class GeometryShader :public Shader {
public:
	GeometryShader(std::string vertexSource, std::string fragmentSource) :Shader(vertexSource, fragmentSource) {}
	GeometryShader(std::string vertexSource, std::string geometrySource, std::string fragmentSource) :Shader(vertexSource, geometrySource, fragmentSource) {}
	// ����ͳһ����model��modelBuffer��Shader������ʹ�ô˺�������
	void setModel(glm::mat4 model) {
		(*this)["model"] = model;
	}
	void setModelBuffer(glm::mat4* modelBuffer, int size) {
		(*this)["modelBuffer"] = modelBuffer;
		(*this)["modelBufferSize"] = size;
	}
};

class DefaultShader :public GeometryShader {	// ��Ĭ��Shader����Ϊ���廯
public:
	DefaultShader() :GeometryShader("shader/shader.gvs", "shader/shader.gfs") {
		(*this)["isAuto"] = true;
	}
	void setColor(glm::vec4 color) {
		(*this)["isAuto"] = false;
		(*this)["ncolor"] = color;
	}
	void setAuto() {
		(*this)["isAuto"] = true;
	}
};


