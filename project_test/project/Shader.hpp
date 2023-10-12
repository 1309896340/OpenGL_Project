#ifndef __WIND_SHADER
#define __WIND_SHADER

#include "utils.h"
#include "proj.h"

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
		this->use();
		ShaderUniform su(glGetUniformLocation(ID, name.c_str()));
		return su;
	}
	// 标准化配置uniform：model、modelBuffer
	void setModel(const glm::mat4& model) {
		(*this)["model"] = model;
	}
	void setModelBuffer(const glm::mat4& modelBuffer) {
		(*this)["modelBuffer"] = modelBuffer;
	}
	// 定制化配置uniform
	virtual void loadAttribute(const uniformTable& attribute) = 0;
	// 使用loadUniform来实现多态行为，在子类中实现根据不同的着色器加载不同的uniform变量
};


class DefaultShader :public Shader {
private:
	DefaultShader() :Shader("shader/shader.gvs", "shader/shader.gfs") {}
public:
	DefaultShader(DefaultShader&) = delete;
	DefaultShader& operator=(DefaultShader&) = delete;
	~DefaultShader() = default;

	static DefaultShader* getDefaultShader() {
		static DefaultShader* shader = nullptr;
		if (shader == nullptr) {
			shader = new DefaultShader();
		}
		return shader;
	}

	virtual void loadAttribute(const uniformTable& attribute) {
		// 默认着色器定制uniform为颜色配置
		(*this)["ncolor"] = attribute.color;
		(*this)["isAuto"] = attribute.autoColor;
	}
};


class ComputeShader : public Shader {
public:
	ComputeShader(std::string computePath) :Shader() {
		compileAndAttachShader(computePath, GL_COMPUTE_SHADER);
		link();
	}
	~ComputeShader() {
		glDeleteProgram(ID);
	}

};

#endif
