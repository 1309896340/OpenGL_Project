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
			std::cout << "返回未链接成功的shader！" << std::endl;	// 调试输出
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
	void use() {		// 在glDrawXXX()前务必记得调用此函数
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
		//this->use();
		ShaderUniform su(glGetUniformLocation(ID, name.c_str()));
		return su;
	}
	// 标准化配置uniform：model、modelBuffer
	//void setModel(const mat4& model) {
	//	(*this)["model"] = model;
	//}
	//void setModelBuffer(const mat4& modelBuffer) {
	//	(*this)["modelBuffer"] = modelBuffer;
	//}
	// 定制化配置uniform
	//void loadAttribute(const uniformTable& attribute) {
	//	// 默认着色器定制uniform为颜色配置
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
	// 计算着色器需要进行手动调度，传入工作组数量，并同步内存
	void dispatch(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {
		this->use();
		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
};

#endif
#endif