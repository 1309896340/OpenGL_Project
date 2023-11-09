#include "proj.h"

#ifdef TEST_OPENGL
#ifndef __WIND_SHADER
#define __WIND_SHADER

#include "utils.h"

class ShaderUniform {
private:
	GLint location;
public:
	ShaderUniform(GLint location) :location(location) {}
	void operator=(vec4 value) {
		glUniform4fv(location, 1, value_ptr(value));
	}
	void operator=(vec3 value) {
		glUniform3fv(location, 1, value_ptr(value));
	}
	void operator=(mat4 value) {
		glUniformMatrix4fv(location, 1, GLFW_FALSE, value_ptr(value));
	}
	void operator=(bool value) {
		glUniform1i(location, (int)value);
	}
};

class Shader {
protected:
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
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLchar infoLog[512];
			GLsizei length;
			glGetShaderInfoLog(shader, 512, &length, infoLog);
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
		//this->use();
		ShaderUniform su(glGetUniformLocation(ID, name.c_str()));
		return su;
	}
	// ��׼������uniform��model��modelBuffer
	//void setModel(const mat4& model) {
	//	(*this)["model"] = model;
	//}
	//void setModelBuffer(const mat4& modelBuffer) {
	//	(*this)["modelBuffer"] = modelBuffer;
	//}
	// ���ƻ�����uniform
	//void loadAttribute(const uniformTable& attribute) {
	//	// Ĭ����ɫ������uniformΪ��ɫ����
	//	(*this)["ncolor"] = attribute.color;
	//	(*this)["isAuto"] = attribute.autoColor;
	//}
};


class ComputeShader : public Shader {
public:
	ComputeShader(std::string computePath) :Shader() {
		compileAndAttachShader(computePath, GL_COMPUTE_SHADER);
		link();
	}
	// ������ɫ����Ҫ�����ֶ����ȣ����빤������������ͬ���ڴ�
	void dispatch(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {
		this->use();
		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
};

#endif
#endif